// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "push/PushLeds.h"
#include "push/PushSysex.h"
#include "push/PushMidi.h"

#include "logger.h"

namespace PushLib {

const std::vector<Button> _allButtons = { 
    Button::TapTempo, Button::Metrnonme, Button::DisplayBottom1, Button::DisplayBottom2, 
    Button::DisplayBottom3, Button::DisplayBottom4, Button::DisplayBottom5, 
    Button::DisplayBottom6, Button::DisplayBottom7, Button::DisplayBottom8, 
    Button::Master, Button::StopClip, Button::Setup, Button::Layout, Button::Convert, 
    Button::Scene14, Button::Scene14t, Button::Scene18, Button::Scene18t, Button::Scene116, 
    Button::Scene116t, Button::Scene132, Button::Scene132t, Button::Left, Button::Right, 
    Button::Up, Button::Down, Button::Select, Button::Shift, Button::Note, Button::Session, 
    Button::AddDevice, Button::AddTrack, Button::OctaveDown, Button::OctaveUp, 
    Button::Repeat, Button::Accent, Button::Scale, Button::User, Button::Mute, Button::Solo, 
    Button::PageLeft, Button::PageRight, Button::Play, Button::Record, Button::New, 
    Button::Duplicate, Button::Automate, Button::FixedLength, Button::DisplayTop1, 
    Button::DisplayTop2, Button::DisplayTop3, Button::DisplayTop4, Button::DisplayTop6, 
    Button::DisplayTop7, Button::DisplayTop8, Button::Device, Button::Browser, Button::Mix, 
    Button::Clip, Button::Quantize, Button::DoubleLoop, Button::Delete, Button::Undo
};

enum LedSysex : unsigned char {
    SET_LED_COLOR_PALETTE_ENTRY = 0x03,
    GET_LED_COLOR_PALETTE_ENTRY = 0x04,
    REAPPLY_COLOR_PALETTE = 0x05,
    SET_LED_BRIGHTNESS = 0x06,
    GET_LED_BRIGHTNESS = 0x07,
    SET_LED_PWM_FREQ_CORRECTION = 0x0B,
    SET_LED_WHITE_BALANCE = 0x14,
    GET_LED_WHITE_BALANCE = 0x15
};

PushLeds::PushLeds(PushMidi &midi, PushSysex &sysex) : _midi(midi), _sysex(sysex) {
    _sysex.registerCmdWithReply(GET_LED_COLOR_PALETTE_ENTRY);
    _sysex.registerCmdWithReply(GET_LED_BRIGHTNESS);
    _sysex.registerCmdWithReply(GET_LED_WHITE_BALANCE);
}

PushLeds::~PushLeds() {

}
/*
void PushLeds::setLedColor(unsigned char lednum, LedAnimation anim, unsigned char color) {
    // _midi.send();
    slr::MidiEvent msg;
    unsigned char *data = (unsigned char*)&msg; //some C hack lol :)
    data[0] = 0; data[1] = 0; data[2] = 0; data[3] = 0; 
    data[4] = 0; data[5] = 0; data[6] = 0; data[7] = 0;
    //msg.type =...
    data[0] = static_cast<unsigned char>(slr::MidiEventType::CC) |
                (static_cast<unsigned char>(anim.type) +
                 static_cast<unsigned char>(anim.duration));

    msg.note = lednum;
    msg.velocity = color;

    _midi.sendSingle(msg);
}

void PushLeds::setLedColors(std::vector<unsigned char> &lednums, LedAnimation anim, unsigned char clr) {
    std::vector<slr::MidiEvent> batch;

    for(auto & num : lednums) {
        slr::MidiEvent ev;
        {
            unsigned char *data = (unsigned char*)&ev; //some C hack lol :)
            data[0] = 0; data[1] = 0; data[2] = 0; data[3] = 0; 
            data[4] = 0; data[5] = 0; data[6] = 0; data[7] = 0;
            //msg.type =...
            data[0] = static_cast<unsigned char>(slr::MidiEventType::CC) |
                        (static_cast<unsigned char>(anim.type) +
                        static_cast<unsigned char>(anim.duration));
        }
        ev.note = num;
        ev.velocity = clr;
        batch.push_back(ev);
    }
    _midi.sendBatch(batch);
}

void PushLeds::clearAllLeds() {

}*/

void PushLeds::setButtonColor(Button &btn, unsigned char &color) {
    slr::MidiEvent ev;
    ev.type = slr::MidiEventType::CC;
    ev.note = static_cast<uint8_t>(btn);
    ev.velocity = static_cast<uint8_t>(color);
    ev.channel = 0;
    ev.offset = 0;
    _midi.sendSingle(ev);
}

void PushLeds::setButtonsColor(std::vector<std::pair<Button, unsigned char>> &btns) {
    std::vector<slr::MidiEvent> batch;
    std::size_t size = btns.size();
    batch.reserve(size);

    for(std::size_t b=0; b<size; ++b) {
        std::pair<Button, unsigned char> &btn = btns[b];
        slr::MidiEvent ev;
        ev.type = slr::MidiEventType::CC; //TODO: Add LedAnimation
        ev.note = static_cast<uint8_t>(btn.first);
        ev.velocity = static_cast<uint8_t>(btn.second);
        ev.channel = 0;
        ev.offset = 0;
    }

    _midi.sendBatch(batch);
}

void PushLeds::clearAllButtons() {
    std::vector<slr::MidiEvent> batch;
    std::size_t size = _allButtons.size();
    batch.reserve(size);

    for(std::size_t b=0; b<size; ++b) {
        slr::MidiEvent ev;
        ev.type = slr::MidiEventType::CC; //TODO: add LedAnimation
        ev.note = static_cast<uint8_t>(_allButtons[b]);
        ev.velocity = 0;
        ev.channel = 0;
        ev.offset = 0;
    }

    _midi.sendBatch(batch);
}

void PushLeds::setPadColor(const Pad &pad) {
    if(pad.num < 36 || pad.num > 99) {
        LOG_ERROR("Wrond pad num!");
        return;
    }

    if(pad.color < 0 || pad.color > 127) {
        LOG_ERROR("Wrond color index!");
        return;
    }

    slr::MidiEvent ev;
    ev.type = slr::MidiEventType::NoteOn; ////TODO: +LedAnimation
    ev.note = static_cast<uint8_t>(pad.num);
    ev.velocity = static_cast<uint8_t>(pad.color);
    ev.channel = 0; //put animation here?
    ev.offset = 0;
    LOG_INFO("%x %x %x %x %x", (int)ev.type, (int)ev.note, (int)ev.velocity, (int)ev.channel, (int)ev.offset);
    _midi.sendSingle(ev);
}

void PushLeds::setPadsColor(const std::vector<Pad> &pads) {
    if(pads.size() != 64) {
        LOG_ERROR("Wrong vector size!");
        return;
    }
    
    std::vector<slr::MidiEvent> batch;
    batch.reserve(64);

    for(int i=0; i<64; ++i) {
        const Pad &itm = pads.at(i);
        slr::MidiEvent ev;
        ev.type = slr::MidiEventType::NoteOn; //TODO: +LedAnimation
        ev.note = static_cast<uint8_t>(itm.num);
        ev.velocity = static_cast<uint8_t>(itm.color);
        ev.channel = 0;
        ev.offset = 0;
        batch.push_back(ev);
    }

    _midi.sendBatch(batch);
}

void PushLeds::clearAllPads() {
    std::vector<slr::MidiEvent> batch;
    batch.reserve(64);

    for(int i=36; i<99; ++i) {
        slr::MidiEvent ev;
        ev.type = slr::MidiEventType::NoteOn;
        ev.note = static_cast<uint8_t>(i);
        ev.velocity = 0;
        ev.channel = 0;
        ev.offset = 0;
        batch.push_back(ev);
    }

    _midi.sendBatch(batch);
}




}