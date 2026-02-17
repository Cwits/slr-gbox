// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "push/PushCore.h"

#include "core/primitives/MidiPort.h"
#include "core/MidiController.h"
#include "core/Events.h"

#include "logger.h"

#include <thread>
#include <atomic>
#include <future>

namespace PushLib {

void pushThreadLoop(PushLib::PushCore * push);

std::thread _pushThread;
std::atomic<bool> _threadRunning;

PushCore::PushCore() {

}

PushCore::~PushCore() {

}

bool PushCore::connect(slr::MidiDevice *dev, slr::MidiSubdevice *subdev, slr::MidiPort * port) {
    _dev = dev;
    _subdev = subdev;
    _port = port;
    
    _pushThread = std::thread(&pushThreadLoop, this);

    return true;
}

void PushCore::reconnect() {
    
}

void PushCore::disconnect() {
    _threadRunning = false;
    _pushThread.join();
}

void PushCore::handleEvent(const slr::MidiEventType type, const int channel, const unsigned char *data) {
    LOG_INFO("Event from Push");
}

void PushCore::handleRealTimeEvent(const slr::MidiEventType type) {
    // LOG_INFO("RTEvent from Push");
}

void PushCore::handleSysexEvent(const unsigned char *data, const int &len) {
    LOG_INFO("sysex Event from Push");
}


void pushThreadLoop(PushLib::PushCore * push) {
    //... some preps
    push->_port->setAlternativeHandles(
        [push](slr::MidiEventType t){
            push->handleRealTimeEvent(t);
        },
        [push](const unsigned char *data, const int &len){
            push->handleSysexEvent(data, len);
        },
        [push](slr::MidiEventType t, int ch, const unsigned char *data){
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
        .device = push->_dev,
        .subdev = push->_subdev,
        .port = slr::DevicePort::INPUT,
        .newState = true,
        .completed = [bptr = &trigg1, result](int res) mutable {
                        LOG_INFO("Midi Device toggled res %d", res);
                        result = res;
                        bptr->store(true);
                    }
    };
    slr::EmitEvent(e);
    
    e.port = slr::DevicePort::OUTPUT;
    e.completed = [bptr = &trigg2, result2](int res) mutable {
        LOG_INFO("Midi Device toggled res %d", res);
        result2 = res;
        bptr->store(true);
    };
    slr::EmitEvent(e);

    while(!trigg1 && !trigg2) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if(result != 0 && result2 != 0) {
        LOG_ERROR("Failed to init Push Midi communication");
        return;
    }

    if(!push->_display.connect()) {
        LOG_ERROR("Failed to connect Push display");
        return;
    }


    LOG_INFO("Push Inited successfully");
    _threadRunning = true;
    // preps
    while(_threadRunning) {
        push->_display.sendFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}



}