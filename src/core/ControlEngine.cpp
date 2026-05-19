// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/ControlEngine.h"

#include "core/drivers/AudioDriver.h"

#include "core/primitives/ControlContext.h"
#include "core/primitives/SPSCQueue.h"
#include "core/primitives/ActionBase.h"
#include "core/primitives/ActionExecutable.h"
#include "core/primitives/MidiEvent.h"
#include "core/primitives/RtTask.h"

#include "core/ActionsMap.h"
// #include "core/AudioBufferManager.h"
#include "core/BufferManager.h"
#include "core/RtEngine.h"
#include "core/Project.h"
#include "core/FileWorker.h"
#include "core/SettingsManager.h"
#include "core/UnitManager.h"
#include "core/MidiController.h"
#include "core/Metronome.h"

#include "core/utility/helper.h"

#include "logger.h"

#include "snapshots/ProjectView.h"
#include "snapshots/FileContainerView.h"
#include "snapshots/DriverView.h"

#include "ui/uiControls.h"

#include <cassert>
#include <thread>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <variant>
#include <algorithm>
#include <optional>
#include <condition_variable>
#include <unordered_map>
#include <future>
#include <queue>

#define QUEUE_INITIAL_SIZE 128

namespace slr {

std::thread _controlThread;
std::atomic<bool> _shutdown;
std::condition_variable _cond;

std::unique_ptr<BufferManager> _bufferManager;

std::unique_ptr<RtEngine> _engine;
    
std::unique_ptr<Project> _project;
std::unique_ptr<FileWorker> _fileWorker;

std::unique_ptr<ProjectView> _projectSnapshot;
std::unique_ptr<DriverView> _driverView;

std::thread _midiDiscoverThread;
std::unique_ptr<MidiController> _midiController;

std::vector<std::unique_ptr<ActionExecutable>> _actions;
std::shared_mutex _actionMutex;

std::queue<std::unique_ptr<ActionExecutable>> _undoList;
std::queue<std::unique_ptr<ActionExecutable>> _redoList;

// ID _commandIdCounter = 0;

namespace ControlEngine {

void discoverMidi();

ActionExecutable * getAction(std::size_t &index) {
    std::shared_lock l(_actionMutex);

    ActionExecutable *ret = nullptr;
    if(_actions.size() > index) {
        ret = _actions.at(index).get();
        index++;
    }
    return ret;
}

void processLoop() {
    bool pendingDeleteEvent = false;

    while(!_shutdown) {
        ControlContext ctx(_project.get(),
                            _fileWorker.get(),
                            _engine.get(),
                            _projectSnapshot.get(),
                            _midiController.get(),
                            _bufferManager.get());

        if(_engine == nullptr || _engine->getState() != RtEngine::RtState::RUN) {
            LOG_WARN("RT Engine not ready!");
            // goto sleep;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        std::size_t idx = 0;
        ActionExecutable * action = nullptr;
        while( (action = getAction(idx)) != nullptr ) {
            if(action->toDelete()) continue;

            switch(action->getState()) {
                case(ActionState::Executing): action->exec(ctx); break;
                case(ActionState::Waiting): action->checkWaitingCondition(ctx); break;
                case(ActionState::Finished): assert(false && "Shouldn't be here"); break;
            }
        } 

        {
            //cleaning actions
            //check if action is finished
            //if finished - check if action can be undoable
            //if can - put it into undo list
            //if can't - just delete
            
            
            std::unique_lock l(_actionMutex);
            _actions.erase(
                std::remove_if(
                    _actions.begin(),
                    _actions.end(),
                    [](const std::unique_ptr<ActionExecutable> &a) {
                        return a->toDelete();
                    }
                ),
                _actions.end()
            );
        }

        {
            //check for responses from RT
            SPSCQueue<RtTask*, 256> & resps = _engine->getResponses();
            
            RtTask * task = nullptr;
            while(resps.pop(task)) {
                task->fn(task->obj);
            }
        }
        //TODO:check pools for need for expand:
        //e.g. if audiobufferpool::regularsize < 8 than expand
        //      or recordsize < 16 expand
        //...

        // checkMidiDevices();

        // sleep:        
        // std::unique_lock<std::mutex> l(_this->_controlLock);
        // if(_this->_cond.wait_for(l, std::chrono::milliseconds(100)) == std::cv_status::timeout) {
        //     // if(f->_shutdown) {
        //     //     goto exit;
        //     // }
        // }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}


bool init() {
    SettingsManager::init();

    _shutdown = false;

    _bufferManager = std::make_unique<BufferManager>();
    if(!_bufferManager->init(SettingsManager::getBlockSize(), DEFAULT_BUFFER_CHANNELS)) {
        LOG_ERROR("Failed to init Buffer Manager");
        return false;
    }

    // if(!AudioBufferManager::init(SettingsManager::getBlockSize(),
    //                             DEFAULT_BUFFER_CHANNELS)) {
    //     LOG_ERROR("Failed to init Audio Buffer Manager");
    //     return false;
    // }

    _fileWorker = std::make_unique<FileWorker>();
    if(!_fileWorker->init()) {
        LOG_ERROR("Failed to init File Worker");
        return false;
    }

    _engine = std::make_unique<RtEngine>();
    if(!_engine->init()) {
        LOG_ERROR("Failed to init RT Engine");
        return false;
    }

    UnitManagerFactory::init();

    _midiController = std::make_unique<MidiController>();

    _controlThread = std::thread(ControlEngine::processLoop);

    _driverView = std::make_unique<DriverView>(_engine->driver());

    /*
        if loagind project than do it here
    */

    _project = std::make_unique<Project>();
    _project->metronome()->create(_bufferManager.get());
    _projectSnapshot = std::make_unique<ProjectView>(&_project->timeline());

    _engine->setProject(_project.get());
    if(!_engine->start([ctl = _midiController.get()](frame_t framesPassed) {
        ctl->setAnchor(framesPassed);
    })) {
        LOG_ERROR("Failed to start RT Engine");
        return false;
    }
    
    _midiDiscoverThread = std::thread(ControlEngine::discoverMidi);

    return true;
}

bool shutdown() {
    _shutdown = true;
    if(!_engine->stop()) {
        LOG_ERROR("Failed to stop RT Engine at shutdown");
        return false;
    }

    _engine.reset();

    /* save project? */
    _projectSnapshot.reset();
    _project.reset();
    
    if(!_fileWorker->shutdown()) {
        LOG_ERROR("Failed to shutdown File Worker");
        return false;
    }
    _fileWorker.reset();

    // if(!AudioBufferManager::shutdown()) {
    //     LOG_ERROR("Failed to shutdown Audio Buffer Manager");
    //     return false;
    // }

    _bufferManager->shutdown();

    _midiController.reset();

    _controlThread.join();
    _midiDiscoverThread.join();

    return true;
}
void emergencyStop() {

}

void prepareForProjectLoading() {
    if(!_engine->stop()) {
        LOG_FATAL("Failed to stop Engine");
    }

    UIControls::clearUI();

    //TODO: need some method to clear some global data(ed ID counters and etc...);
    _projectSnapshot.reset();
    _project.reset();

    LOG_ERROR("Enable this two guys");
    // _fileWorker->clear();
    // _bufferManager->clear();


    _project = std::make_unique<Project>();
    _project->metronome()->create(_bufferManager.get());
    _projectSnapshot = std::make_unique<ProjectView>(&_project->timeline());

    _engine->setProject(_project.get());
    if(!_engine->start([ctl = _midiController.get()](frame_t framesPassed) {
        ctl->setAnchor(framesPassed);
    })) {
        LOG_ERROR("Failed to start RT Engine");
        return;
    }
    

}

void EmitAction(std::unique_ptr<ActionBase> action) {
    const std::map<std::type_index, CreatorFn> &map = getActionMap();
    assert(map.count(action->actionType()));

    std::unique_ptr<ActionExecutable> actexe = map.at(action->actionType())(action.get());

    std::unique_lock l(_actionMutex);
    _actions.push_back(std::move(actexe));
}

ProjectView * projectSnapshot() {
    return _projectSnapshot.get();
}

DriverView * driverSnapshot() {
    return _driverView.get();
}

RtEngine * rtEngine() {
    return _engine.get();
}

FileWorker * fileWorker() {
    return _fileWorker.get();
}

MidiController * midiController() {
    return _midiController.get();
}

BufferManager * bufferManager() {
    return _bufferManager.get();
}

void discoverMidi() {
    while(!_shutdown) {
        _midiController->checkDevices();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}


} //namespace ControlEngine

} //namespace slr