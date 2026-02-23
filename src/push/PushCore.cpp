// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "push/PushCore.h"

#include "core/primitives/MidiPort.h"
#include "core/MidiController.h"
#include "core/Events.h"

#include "push/Widget.h"

#include "logger.h"
#include "slr_config.h"

#include <thread>
#include <atomic>
#include <future>
#include <chrono>
#include <cmath>

#if (USE_FAKE_PUSH == 1)
#include "push/PushFaker.h"
PushLib::PushFaker _faker;
#endif


namespace PushLib {

PushCore::PushCore() :
    _painter(Painter()),
    _display(DisplayInterface(_painter)),
    _mainWidget(nullptr),
    _manualRedraw(false)
{

}

PushCore::~PushCore() {

}

bool PushCore::connect(slr::MidiPort * port) {
    _port = port;

#if (USE_FAKE_PUSH == 1)

    LOG_WARN("Using fake Push");
    _faker.realTimeHandle = [push = this](const unsigned char t) {
        push->handleRealTimeEvent(static_cast<slr::MidiEventType>(t));
    };
    _faker.sysexHandle = [push = this](const unsigned char *data, const int &len){
        push->handleSysexEvent(data, len);
    };
    _faker.eventHandle = [push = this](const unsigned char t, const int ch, const unsigned char *data){
        push->handleEvent(static_cast<slr::MidiEventType>(t), ch, data);
    };

    _faker.init();

#endif
    _connected = _display.connect();
    return true;
}

void PushCore::reconnect() {
    
}

void PushCore::disconnect() {
    #if (USE_FAKE_PUSH == 0)
    _display.disconnect();
    #else
    _faker.deinit();
    #endif
}

void PushCore::handleEvent(const slr::MidiEventType type, const int channel, const unsigned char *data) {
    // LOG_INFO("CC Event");
    //comes from another thread!
    /*
    //if pad than process asap and pass to _port->spscqueue?
    //and how than we handle audiounit switch? like... sending event every time? 
    it will require to rebuild Render Plan in order to enable reading from spsc... and that is highly unwanted...
    than we have a inject method... that's seems possible, but need to somehow keep things in sync then...
    so use same thing as in MidiPort::pushEvent, but for injecting... 
    */
    switch(type) {
        case(slr::MidiEventType::NoteOn):
        case(slr::MidiEventType::NoteOff): {
            //pad pressed
            LOG_WARN("Not supported yet");
            //get pad num(midi note) as soon as possible and send to target.
            //it must be thread safe...
        } break;
        case(slr::MidiEventType::Aftertouch): {
            //i'll guess this one works as well for pads
            LOG_WARN("Not supported yet");
        } break;
        case(slr::MidiEventType::PitchBend): {
            //
            LOG_WARN("Not supported yet");
        } break;
        case(slr::MidiEventType::ChannelPressure): {
            //not sure
            LOG_WARN("Not supported yet");
        } break;
        case(slr::MidiEventType::CC): {
            //button or encoder
            // IncomeEvent inEv;
            int id = static_cast<int>(data[0]);
            int value = static_cast<int>(data[1]);
            if(id == 14 || id == 15 || (id >= 71 && id <= 79)) {
                //encoder
                EncoderEvent e;
                e.type = static_cast<EncoderEventType>(channel);
                e.encoder = static_cast<Encoder>(id);
                e.delta = value;
                e.raw = 0;
            } else {
                //button
                ButtonEvent b;
                b.button = static_cast<Button>(id);
                b.type = static_cast<ButtonEventType>(value);
                std::lock_guard<std::mutex> l(_pushMutex);
                _buttonQueue.push(b);
            }

        } break;
        case(slr::MidiEventType::InvalidType):
        case(slr::MidiEventType::ProgramChange):
        case(slr::MidiEventType::SysEx):
        case(slr::MidiEventType::TimeCodeQuarterFrame):
        case(slr::MidiEventType::SongPosition):
        case(slr::MidiEventType::SongSelect):
        case(slr::MidiEventType::TuneRequest):
        case(slr::MidiEventType::Clock):
        case(slr::MidiEventType::Start):
        case(slr::MidiEventType::Continue):
        case(slr::MidiEventType::Stop):
        case(slr::MidiEventType::ActiveSensing):
        case(slr::MidiEventType::SystemReset):
        default:
            LOG_INFO("Shouldn't be here");
        break;
    }
}

void PushCore::handleRealTimeEvent(const slr::MidiEventType type) {
    //comes from another thread!
    // LOG_INFO("RTEvent from Push");
}

void PushCore::handleSysexEvent(const unsigned char *data, const int &len) {
    //comes from another thread!
    LOG_INFO("sysex Event from Push");
}

void PushCore::tick(int dt) {
    bool needRedraw = dispatchEvents();

    //animations tick?
    
    if(needRedraw || _manualRedraw) {
        _manualRedraw = false;
        if(_mainWidget) _mainWidget->paint(_painter);
        else LOG_WARN("Main Widget is not set");
        // _painter.paint();
        _display.updateFrame();
    }

    if(!_display.sendFrame()) 
        _connected = false;
}

bool PushCore::dispatchEvents() {
    {
        std::lock_guard<std::mutex> l(_pushMutex);
        _localButtonQueue.swap(_buttonQueue);
    }

    bool ret = false;
    while(!_localButtonQueue.empty()) {
        ButtonEvent ev = _localButtonQueue.front();
        _localButtonQueue.pop();

        if(_mainWidget)
            ret = _mainWidget->handleButton(ev);
    }

    return ret;
}


}