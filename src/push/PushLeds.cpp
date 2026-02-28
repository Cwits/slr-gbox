// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "push/PushLeds.h"
#include "push/PushSysex.h"
#include "push/PushMidi.h"

namespace PushLib {

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
    // _midi.sendBatch(batch);
}

void PushLeds::clearAllLeds() {

}


}