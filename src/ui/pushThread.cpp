// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/pushThread.h"

#include "core/primitives/MidiPort.h"
#include "core/MidiController.h"
#include "core/Events.h"

#include "push/PushCore.h"
#include "ui/push/MainWindow.h" //does not belong here...

#include "slr_config.h"
#include "logger.h"

#include <thread>
#include <atomic>
#include <memory>
#include <chrono>

namespace PushThread {

std::unique_ptr<PushLib::PushCore> _pushDev;
std::unique_ptr<PushUI::MainWindow> _pushMainWindow;

std::thread _pushRunner;
std::atomic<bool> _threadRunning;

void threadLoop(slr::MidiPort * port);

void start(slr::MidiPort * port) {
    if(_threadRunning) {
        LOG_WARN("Push thread already running");
        return;
    }

    _pushRunner = std::thread(&threadLoop, port);
}

void shutdown() {
    _pushDev->disconnect();

    _threadRunning = false;
    _pushRunner.join();

    _pushDev.reset();
}

bool isRunning() {
    return _threadRunning;
}

void threadLoop(slr::MidiPort *port) {
    _pushDev.reset();
    _pushMainWindow.reset();

    _pushDev = std::make_unique<PushLib::PushCore>();
    _pushMainWindow = std::make_unique<PushUI::MainWindow>();

    _pushDev->setMainWidget(_pushMainWindow.get());

#if (USE_FAKE_PUSH == 0)
    //... some preps
    port->setAlternativeHandles(
        [push = _pushDev.get()](const slr::MidiEventType t){
            push->handleRealTimeEvent(t);
        },
        [push = _pushDev.get()](const unsigned char *data, const int &len){
            push->handleSysexEvent(data, len);
        },
        [push = _pushDev.get()](const slr::MidiEventType t, const int ch, const unsigned char *data){
            push->handleEvent(t, ch, data);
        }
    );

    /*
        ugh... this so terrible... 
    */
    std::atomic<bool> trigg1 = false;
    std::atomic<bool> trigg2 = false;
    int result = 0;
    int result2 = 0;

    slr::Events::ToggleMidiDevice e = {
        .device = port->_dev,
        .subdev = port->_subdev,
        .port = slr::DevicePort::INPUT,
        .newState = true,
        .completed = [bptr = &trigg1, result](int res) mutable {
                        LOG_INFO("Midi Device toggled res %d", res);
                        result = res;
                        bptr->store(true);
                    }
    };
    slr::EmitEvent(e);

    // if(!slr::ControlEngine::awaitEventBlocking(e, 1000*5)) {
    //     LOG_ERROR("Failed to enable Input midi port for push");
    //     return;
    // }
    
    e.port = slr::DevicePort::OUTPUT;
    e.completed = [bptr = &trigg2, result2](int res) mutable {
        LOG_INFO("Midi Device toggled res %d", res);
        result2 = res;
        bptr->store(true);
    };
    slr::EmitEvent(e);

    // if(!slr::ControlEngine::awaitEventBlocking(e, 1000*5)) {
    //     LOG_ERROR("Failed to enable Output midi port for push");
    //     return; 
    // }

    while(!trigg1 && !trigg2) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if(result != 0 && result2 != 0) {
        LOG_ERROR("Failed to init Push Midi communication");
        return;
    }
#endif

    if(!_pushDev->connect(port)) {
        LOG_ERROR("Failed to connect Push display");
        return;
    }

    LOG_INFO("Push Inited successfully");
    _threadRunning = true;

    int FPS = 30.0;
    std::chrono::milliseconds dt(1000/FPS);
    std::chrono::time_point<std::chrono::steady_clock> lastFrame = std::chrono::steady_clock::now();
    while(_threadRunning) {
        if(_pushDev->connected()) {
            auto now = std::chrono::steady_clock::now();

            unsigned long diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFrame).count();
            _pushDev->tick(diff);

            lastFrame = now;
        } else {
            //try reconnect
            //ugh... than midi would be disconnected as well, need some mechanism in MidiController to reconnect midi
        }

        std::this_thread::sleep_for(dt);
    }
}

}