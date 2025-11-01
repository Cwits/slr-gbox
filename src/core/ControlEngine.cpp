// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/ControlEngine.h"

#include "core/drivers/AudioDriver.h"

#include "core/primitives/ControlContext.h"

#include "core/AudioBufferManager.h"
#include "core/Events.h"
#include "core/FlatEvents.h"
#include "core/RtEngine.h"
#include "core/Project.h"
#include "core/FileWorker.h"
#include "core/SettingsManager.h"
#include "core/CEHandlerTables.h"
#include "core/ModuleManager.h"

#include "core/utility/helper.h"

#include "logger.h"

#include "snapshots/ProjectView.h"
#include "snapshots/FileContainerView.h"
#include "snapshots/DriverView.h"

#include "ui/uiControls.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <variant>
#include <algorithm>
#include <optional>
#include <condition_variable>
// #include <libudev.h>

#define QUEUE_INITIAL_SIZE 128

namespace slr {

std::thread _controlThread;
std::atomic<bool> _shutdown;
std::condition_variable _cond;

std::vector<Events::Event> _eventQueue;
std::vector<Events::Event> _eventSnapshot;
std::mutex _controlLock;

std::unique_ptr<RtEngine> _engine;
    
std::unique_ptr<Project> _project;
std::unique_ptr<FileWorker> _fileWorker;

std::unique_ptr<ProjectView> _projectSnapshot;
std::unique_ptr<DriverView> _driverView;

struct awaitEvent {
    FlatEvents::FlatControl ctl;
    std::function<void(const ControlContext&, const FlatEvents::FlatResponse&)> fire;
    bool deleteEvent = false;
};

std::vector<awaitEvent> _awaitEvents;

ID _commandIdCounter = 0;

// udev * _udevctx;

namespace ControlEngine {
void processLoop();

bool init() {
    SettingsManager::init();

    // _udevctx = udev_new();

    _shutdown = false;
    _eventQueue.reserve(QUEUE_INITIAL_SIZE);
    _eventSnapshot.reserve(QUEUE_INITIAL_SIZE);

    if(!AudioBufferManager::init(SettingsManager::getBlockSize(),
                                DEFAULT_BUFFER_CHANNELS)) {
        LOG_ERROR("Failed to init Audio Buffer Manager");
        return false;
    }

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

    ModuleManagerFactory::init();

    _controlThread = std::thread(ControlEngine::processLoop);

    _driverView = std::make_unique<DriverView>(_engine->driver());

    /*
        if loagind project than do it here
    */

    _project = std::make_unique<Project>();
    _projectSnapshot = std::make_unique<ProjectView>(&_project->timeline());

    _engine->setProject(_project.get());
    if(!_engine->start()) {
        LOG_ERROR("Failed to start RT Engine");
        return false;
    }

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
    _project.reset();
    
    if(!_fileWorker->shutdown()) {
        LOG_ERROR("Failed to shutdown File Worker");
        return false;
    }
    _fileWorker.reset();

    if(!AudioBufferManager::shutdown()) {
        LOG_ERROR("Failed to shutdown Audio Buffer Manager");
        return false;
    }

    _controlThread.join();

    // udev_unref(_udevctx);
    return true;
}
void emergencyStop() {

}

const ID generateCommandId() {
    return _commandIdCounter++;
}

void EmitEvent(const Events::Event &e) {
    std::lock_guard<std::mutex> l(_controlLock);
    _eventQueue.push_back(e);
}

void emitRtControl(const FlatEvents::FlatControl &ctl) {
    _engine->FlatControlEvent(ctl);
}

void emitRtResponse(const FlatEvents::FlatResponse &resp) {
    _engine->FlatResponseEvent(resp);
}

void awaitRtResult(const FlatEvents::FlatControl &ctl,
                std::function<void(const ControlContext&, 
                    const FlatEvents::FlatResponse&)> clb) 
{
    awaitEvent ev;
    ev.ctl = ctl;
    ev.fire = std::move(clb);
    ev.deleteEvent = false;

    _awaitEvents.push_back(ev);
    emitRtControl(ctl);
}

void notify() {

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



void processLoop() {
    while(!_shutdown) {
        if(_engine == nullptr) {
            LOG_WARN("RT Engine not set!");
            goto sleep;
        }

        if(_engine->getState() != RtEngine::RtState::RUN) {
            LOG_WARN("RT Engine not running!");
            goto sleep;
        }

        {
            std::lock_guard<std::mutex> l(_controlLock);
            _eventSnapshot = _eventQueue;
            _eventQueue.clear();
        }

        ControlContext ctx;
        ctx.project = _project.get();
        ctx.fileWorker = _fileWorker.get();
        ctx.engine = _engine.get();
        ctx.projectView = _projectSnapshot.get();

        //handle general Events
        for(Events::Event &e : _eventSnapshot) {
            Handlers::ControlTable[e.index()](ctx, e);
        }
        _eventSnapshot.clear();

        //handle Responses from RT Engine
        {
            FlatEvents::FlatResponse resp;
            SPSCQueue<FlatEvents::FlatResponse, 256> &queue = _engine->getOutputQueue();
            while(queue.pop(resp)) {
                Handlers::ResponseTable[static_cast<size_t>(resp.type)](ctx, resp);

                for(awaitEvent &e : _awaitEvents) {
                    if(e.ctl.commandId == resp.commandId) {
                        e.fire(ctx, resp);
                        e.deleteEvent = true;
                    }
                }

                _awaitEvents.erase(
                    std::remove_if(
                        _awaitEvents.begin(), 
                        _awaitEvents.end(),
                        [](const awaitEvent &ev) {
                            return ev.deleteEvent;
                        }), 
                    _awaitEvents.end()
                );
            }
        }

        
        //TODO:check pools for need for expand:
        //e.g. if audiobufferpool::regularsize < 8 than expand
        //      or recordsize < 16 expand
        //...

        sleep:        
        // std::unique_lock<std::mutex> l(_this->_controlLock);
        // if(_this->_cond.wait_for(l, std::chrono::milliseconds(100)) == std::cv_status::timeout) {
        //     // if(f->_shutdown) {
        //     //     goto exit;
        //     // }
        // }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// void aggregateEvents() {

// }

void checkMidiDevices() {

}

} //namespace ControlEngine

} //namespace slr