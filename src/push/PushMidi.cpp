// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "push/PushMidi.h"
#include "push/PushCore.h"
#include "push/PushSysex.h"
#include "push/Widget.h"

#include "core/primitives/MidiPort.h"
#include "core/primitives/MidiEvent.h"
#include "core/Events.h"

#include "slr_config.h"
#include "logger.h"


#if (USE_FAKE_PUSH == 1)
#include "push/PushFaker.h"
extern PushLib::PushFaker _faker;
#endif


namespace PushLib {

PushMidi::PushMidi(PushCore &core, PushSysex &sysex) :
    _core(core),
    _sysex(sysex)
{

}

PushMidi::~PushMidi() {

}

bool PushMidi::connect(slr::MidiPort * port) {
    _port = port;

#if (USE_FAKE_PUSH == 0)
    //... some preps
    port->setAlternativeHandles(
        [this](const slr::MidiEventType t){
            this->handleRealTimeEvent(t);
        },
        [this](const unsigned char *data, const int &len){
            this->handleSysexEvent(data, len);
        },
        [this](const slr::MidiEventType t, const int ch, const unsigned char *data){
            this->handleEvent(t, ch, data);
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
        .device = port->_ownerDev,
        .subdev = port->_ownerSubdev,
        .port = slr::DevicePort::INPUT,
        .newState = true,
        .completed = [bptr = &trigg1, result](int res) mutable {
                        LOG_INFO("Midi Device toggled res %d", res);
                        result = res;
                        bptr->store(true);
                    }
    };
    slr::EmitEvent(e);

    // if(!slr::EmitEventBlocking(e, 1000*5)) {
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

    // if(!slr::EmitEventBlocking(e, 1000*5)) {
    //     LOG_ERROR("Failed to enable Output midi port for push");
    //     return; 
    // }

    while(!trigg1 && !trigg2) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if(result != 0 && result2 != 0) {
        LOG_ERROR("Failed to init Push Midi communication");
        return false;
    }
#else
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
    return true;
}

bool PushMidi::reconnect() {
    return true;
}

bool PushMidi::disconnect() {

#if (USE_FAKE_PUSH == 1)
    _faker.deinit();
#endif
    //TODO: probably should get those MIDI disconnected as well??

    return true;
}

void PushMidi::sendSingle(slr::MidiEvent &ev) {
#if (USE_FAKE_PUSH == 0)
    _port->sendMidi(ev);
#else
    if(ev.type >= slr::MidiEventType::SysEx) return;

    unsigned char data[2] = {0};
    data[0] = ev.note;
    data[1] = ev.velocity;
    PushFaker::FakerCmd cmd;
    if(ev.type == slr::MidiEventType::CC) {
        cmd = PushFaker::FakerCmd::Button;
    } else {
        cmd = PushFaker::FakerCmd::Pad;
    }
    _faker.sendCmd(cmd, &data[0], 2);
#endif
}

void PushMidi::sendBatch(std::vector<slr::MidiEvent> &v) {
#if (USE_FAKE_PUSH == 0)
    std::size_t size = v.size();
    for(std::size_t i=0; i<size; ++i) {
        _port->sendMidi(v[i]);
    }
#else 
    for(std::size_t i=0; i<v.size(); ++i) {
        sendSingle(v.at(i));
    }
#endif
}


void PushMidi::handleEvent(const slr::MidiEventType type, const int channel, const unsigned char *data) {
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
        case(slr::MidiEventType::NoteOn): {
            int value = data[0];
            if(value >= 0 && value <= 10) {
                //encoder touched/released
                int event = data[1];
                EncoderEvent e;
                e.type = event == 127 ? EncoderEventType::Touched : EncoderEventType::Released;
                e.delta = 0;
                e.raw = 0;
                if(value <=8) {
                    e.encoder = static_cast<Encoder>(value+71);
                } else if(value == 9) e.encoder = Encoder::Metronome;
                else if(value == 10) e.encoder = Encoder::Tempo;
                _encoderQueue.push(e);
            } else {
                //pad
                const slr::MidiEvent ev = _core._pads.handle(type, channel, data);
                if(ev.type != slr::MidiEventType::InvalidType)
                    _port->pushEvent(ev.type, ev.channel, ev.note, ev.velocity);
            }
        } break;
        case(slr::MidiEventType::NoteOff): 
        case(slr::MidiEventType::Aftertouch):
        case(slr::MidiEventType::ChannelPressure): {
            //pad pressed
            // LOG_WARN("Not supported yet");
            const slr::MidiEvent ev = _core._pads.handle(type, channel, data);
            if(ev.type != slr::MidiEventType::InvalidType)
                _port->pushEvent(ev.type, ev.channel, ev.note, ev.velocity);
        } break;
        case(slr::MidiEventType::PitchBend): {
            //
            LOG_WARN("Not supported yet");
        } break;
        case(slr::MidiEventType::CC): {
            //button or encoder
            // IncomeEvent inEv;
            int id = static_cast<int>(data[0]);
            int value = static_cast<int>(data[1]);
            
            //TODO: the other's important CC will be here as well - e.g. sustain pedal... smth else?
            if(id == 14 || id == 15 || (id >= 71 && id <= 79)) {
                //encoder
                EncoderEvent e;
                e.type = static_cast<EncoderEventType>(channel);
                e.encoder = static_cast<Encoder>(id);
                e.raw = value;
                if(value > 50) {
                    e.delta = -(128-value);
                } else {
                    e.delta = value;
                }
                _encoderQueue.push(e);
            } else {
                //button
                ButtonEvent b;
                b.button = static_cast<Button>(id);
                b.type = value ? ButtonEventType::Pressed : ButtonEventType::Released; 
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

void PushMidi::handleRealTimeEvent(const slr::MidiEventType type) {
    //comes from another thread!
    // LOG_INFO("RTEvent from Push");
}

void PushMidi::handleSysexEvent(const unsigned char *data, const int &len) {
    //comes from another thread!
    // LOG_INFO("sysex Event from Push");
    SysexMsg msg;
    msg.reserve(len);

    for(int i=0; i<len; ++i) {
        msg.push_back(*data);
    }

    _sysexQueue.push(msg);
}


bool PushMidi::dispatchEvents() {
    bool ret = false;
    ButtonEvent btn;
    while(_buttonQueue.pop(btn)) {
        if(_core._mainWidget)
            ret |= _core._mainWidget->handleButton(btn);
    }

    EncoderEvent enc;
    while(_encoderQueue.pop(enc)) {
        if(_core._mainWidget)
            ret |= _core._mainWidget->handleEncoder(enc);
    }

    return ret;
}


}