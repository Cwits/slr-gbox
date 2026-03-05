// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "push/PushPads.h"
#include "push/PushMidi.h"
#include "push/PushSysex.h"
#include "push/PushLeds.h"

#include "logger.h"

namespace PushLib {

enum PadSysex : unsigned char {
    SET_PAD_PARAMETERS = 0x18,
    SET_AFTERTOUCH_MODE = 0x1E,
    GET_AFTERTOUCH_MODE = 0x1F,
    SET_PAD_VELOCITY_CURVE_ENTRY = 0x20,
    GET_PAD_VELOCITY_CURVE_ENTRY = 0x21, //no function for this
    SELECT_PAD_SETTINGS = 0x28,
    GET_SELECTED_PAD_SETTINGS = 0x29, 
    SET_PAD_CALIBRATION = 0x22,
    GET_PAD_CALIBRATION = 0x1D
};

PushPads::PushPads(PushMidi &midi, PushSysex &sysex, PushLeds &leds) :
    _midi(midi),
    _sysex(sysex),
    _leds(leds),
    _needUpdate(false)
{
    _sysex.registerCmdWithReply(GET_AFTERTOUCH_MODE);
    _sysex.registerCmdWithReply(GET_SELECTED_PAD_SETTINGS);
    _sysex.registerCmdWithReply(GET_PAD_VELOCITY_CURVE_ENTRY);
    _sysex.registerCmdWithReply(GET_PAD_CALIBRATION);
}

PushPads::~PushPads() {

}

const slr::MidiEvent PushPads::handle(const slr::MidiEventType &type, const int &channel, const unsigned char *data) {
    /*
     Warning! We still in MidiPort::inputHandle thread!!!
    */
    slr::MidiEvent ret;
    ret.type = slr::MidiEventType::InvalidType;
    if(!_layout) return ret;
    PushPadLayout * lay = _layout.load(std::memory_order_acquire);
    
    switch(type) {
        case(slr::MidiEventType::NoteOn):
        case(slr::MidiEventType::NoteOff):
        case(slr::MidiEventType::Aftertouch): {
            unsigned char pad = data[0];
            unsigned char velocity = data[1];
            
            const unsigned char layoutNote = lay->getNote(pad); 
            
            ret.type = type;
            ret.note = layoutNote;
            ret.velocity = velocity;
            // LOG_INFO("Note played: %d", static_cast<int>(layoutNote));
            lay->renderPad(_leds, pad, type);
        } break;
        case(slr::MidiEventType::ChannelPressure): {
            int pressure = static_cast<int>(data[0]);
            ret.type = type;
            ret.note = 0;
            ret.velocity = pressure;
        } break;
        case(slr::MidiEventType::PitchBend):
        case(slr::MidiEventType::CC):
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
    
    return ret;
}

void PushPads::update() {
    //better move this to PushPadLayout
    if(!_layout) return;
    if(!_needUpdate) return;

    _needUpdate = false;
    _layout.load(std::memory_order_acquire)->renderPads(_leds);
}

}