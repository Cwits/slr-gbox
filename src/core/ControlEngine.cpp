// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/ControlEngine.h"

#include "core/drivers/AudioDriver.h"

#include "core/primitives/ControlContext.h"
#include "core/primitives/SPSCQueue.h"

#include "core/AudioBufferManager.h"
#include "core/Events.h"
#include "core/FlatEvents.h"
#include "core/RtEngine.h"
#include "core/Project.h"
#include "core/FileWorker.h"
#include "core/SettingsManager.h"
#include "core/CEHandlerTables.h"
#include "core/ModuleManager.h"
#include "core/MidiController.h"

#include "core/utility/helper.h"
#include "inc/core/primitives/MidiEvent.h"

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
#include <unordered_map>

#define QUEUE_INITIAL_SIZE 128

namespace slr {

std::thread _controlThread;
std::atomic<bool> _shutdown;
std::condition_variable _cond;

std::unique_ptr<RtEngine> _engine;
    
std::unique_ptr<Project> _project;
std::unique_ptr<FileWorker> _fileWorker;

std::unique_ptr<ProjectView> _projectSnapshot;
std::unique_ptr<DriverView> _driverView;

std::thread _midiDiscoverThread;
std::unique_ptr<MidiController> _midiController;

// SPSCQueue<Events::Event, 256> _controlEventQueue;

std::mutex _controlLock;
std::vector<Events::Event> _eventQueue;
std::vector<Events::Event> _eventSnapshot;

std::vector<FlatEvents::FlatResponse> _flatResponseSnapshot;

struct awaitEvent {
    FlatEvents::FlatControl ctl;
    std::function<void(const ControlContext&, const FlatEvents::FlatResponse&)> fire;
    bool deleteEvent = false;
};

std::vector<awaitEvent> _awaitEvents;

ID _commandIdCounter = 0;

namespace ControlEngine {

void discoverMidi();


void processLoop() {
    bool pendingDeleteEvent = false;

    while(!_shutdown) {
        // Events::Event e;
        if(_engine == nullptr) {
            LOG_WARN("RT Engine not set!");
            goto sleep;
        }

        if(_engine->getState() != RtEngine::RtState::RUN) {
            LOG_WARN("RT Engine not running!");
            goto sleep;
        }

        ControlContext ctx;
        ctx.project = _project.get();
        ctx.fileWorker = _fileWorker.get();
        ctx.engine = _engine.get(); //might be bad :/
        ctx.projectView = _projectSnapshot.get();
        ctx.midiController = _midiController.get();

        //handle general Events
        _eventSnapshot.clear();
        {
            std::lock_guard<std::mutex> l(_controlLock);
            _eventSnapshot = std::move(_eventQueue);
            // _eventQueue.clear();
        }

        for(const Events::Event &e : _eventSnapshot) {
            Handlers::ControlTable[e.index()](ctx, e);
        }

        //handle Responses from RT Engine
        _flatResponseSnapshot.clear();
        {
            FlatEvents::FlatResponse resp;
            SPSCQueue<FlatEvents::FlatResponse, 256> &queue = _engine->getOutputQueue();
            while(queue.pop(resp)) {
                _flatResponseSnapshot.push_back(resp);
            }
        }

        pendingDeleteEvent = false;
        for(FlatEvents::FlatResponse & resp : _flatResponseSnapshot) {
            Handlers::ResponseTable[static_cast<size_t>(resp.type)](ctx, resp);

            for(awaitEvent &e : _awaitEvents) {
                if(e.ctl.commandId == resp.commandId) {
                    e.fire(ctx, resp);
                    e.deleteEvent = true;
                    pendingDeleteEvent = true;
                }
            }
        }

        if(pendingDeleteEvent) {
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
        
        //TODO:check pools for need for expand:
        //e.g. if audiobufferpool::regularsize < 8 than expand
        //      or recordsize < 16 expand
        //...

        // checkMidiDevices();

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


bool init() {
    SettingsManager::init();

    _shutdown = false;
    _eventQueue.reserve(QUEUE_INITIAL_SIZE);
    _eventSnapshot.reserve(QUEUE_INITIAL_SIZE);
    _flatResponseSnapshot.reserve(256);

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

    _midiController = std::make_unique<MidiController>();
    _midiDiscoverThread = std::thread(ControlEngine::discoverMidi);

    _controlThread = std::thread(ControlEngine::processLoop);

    _driverView = std::make_unique<DriverView>(_engine->driver());

    /*
        if loagind project than do it here
    */

    _project = std::make_unique<Project>();
    _projectSnapshot = std::make_unique<ProjectView>(&_project->timeline());

    _engine->setProject(_project.get());
    if(!_engine->start([ctl = _midiController.get()](frame_t framesPassed) {
        ctl->setAnchor(framesPassed);
    })) {
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
    _midiDiscoverThread.join();

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
    // _controlEventQueue.push(e);
}

void emitRtControl(const FlatEvents::FlatControl &ctl) {
    _engine->FlatControlEvent(ctl);
    // _pendingAck.push_back({ctl, false});
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

MidiController * midiController() {
    return _midiController.get();
}

// void aggregateEvents() {

// }

/*
//#include <libudev.h>
void checkMidiDevices() {
    
        // https://github.com/gavv/snippets/blob/master/udev/udev_list_usb_storage.c
        // https://github.com/gavv/snippets/blob/master/udev/udev_monitor_usb.c
        // sooo... for now stay with check via RtMidi::getPortCount();
        // but, for future need to concider possibility that several similar devices will be connected.
        // soo, need to properly map path from udev to name/rtmidi port

        // Прямого API “udev path → ALSA порт” нет. Нужно комбинировать несколько слоёв:

        // 1. get sysfx-path of usb device thru udev
        //     e.g. '/sys/bus/usb/devices/1-1.3/1-1.3:1.0' 
        // 2. find corresponding hw:x,y in ALSA
        //     ALSA creating devices in '/proc/asound/' and '/sys/class/sound/':
            
        //     * `/proc/asound/cards` — map list, index `x`.
        //     * `/proc/asound/seq/clients` — sequential clients ALSA MIDI.
        //     * `/sys/class/sound/cardX/device` → simlink to USB-device (`../../../1-1.3`)

        //     ```
        //     udev sysfs path: /sys/bus/usb/devices/1-1.3
        //     ALSA card path: /sys/class/sound/card1/device -> ../../../1-1.3
        //     ```

        //     Found → `card1` correspond to this USB.

        // 3. MIDI Port
        //     ALSA MIDI-ports on 'cardX' can have several ports('hw:X,0', 'hw:X,1', 'hw:X,0,0').
        //     Can get them via 'snd_seq_get_ports()' or via RtMidi 'getPortName(i)'.

        //     * In `/sys/class/sound/cardX/device` can check interfaces (`midi1`, `midi2`) → correspond with ALSA-ports.

        // ... Summary:
        // 1. From udev path take parent usb-device -> idVendor, idProduct, sysfs path
        // 2. Go through '/sys/class/sound/card* /device' -> find overlaps with USB sysfs path.
        // 3. Find cardX, find corresponding MIDI-ports(0..N) -> correspond with RtMidi ports from 'getPortName()'.
    
   
    udev * _udevctx;
    _udevctx = udev_new();

    struct udev_enumerate* enumerate = udev_enumerate_new(_udevctx);

    udev_enumerate_add_match_subsystem(enumerate, "usb");
    udev_enumerate_add_match_subsystem(enumerate, "usb_interface");
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry *entry;

    
    udev_list_entry_foreach(entry, devices) {
        const char *path = udev_list_entry_get_name(entry);
        struct udev_device *dev = udev_device_new_from_syspath(_udevctx, path);

        if (!dev)
            continue;

        const char *cls = udev_device_get_sysattr_value(dev, "bInterfaceClass");
        const char *subcls = udev_device_get_sysattr_value(dev, "bInterfaceSubClass");

        if (cls && subcls && strcmp(cls, "01") == 0 && strcmp(subcls, "03") == 0) {
            struct udev_device *parent = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
            const char *vendor = udev_device_get_sysattr_value(parent, "idVendor");
            const char *product = udev_device_get_sysattr_value(parent, "idProduct");
            const char *devnode = udev_device_get_devnode(dev);
            LOG_INFO("MIDI device: %s %s %s:%s %s\n",
                    path,
                    udev_device_get_sysattr_value(parent, "product"),
                    vendor ? vendor : "0000",
                    product ? product : "0000",
                    devnode ? devnode : "(no devnode)");
        }

        udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);
    //...
    udev_unref(_udevctx);
}
*/


void discoverMidi() {
    while(!_shutdown) {
        _midiController->checkDevices();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}


} //namespace ControlEngine

} //namespace slr