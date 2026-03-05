// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/PushLib.h"
#include "push/Color.h"
#include <vector>

namespace PushLib {

class PushMidi;
class PushSysex;

struct PushLeds {
    PushLeds(PushMidi &midi, PushSysex &sysex);
    ~PushLeds();

    // void setLedColor(unsigned char lednum, LedAnimation anim, unsigned char color);
    // void setLedColors(std::vector<unsigned char> &lednums, LedAnimation anim, unsigned char clr);
    // void clearAllLeds();

    void setButtonColor(Button &btn, unsigned char &color);
    void setButtonsColor(std::vector<std::pair<Button, unsigned char>> &btns);
    void clearAllButtons();

    void setPadColor(const Pad &pad);
    void setPadsColor(const std::vector<Pad> &pads);
    void clearAllPads();


    private:
    PushMidi &_midi;
    PushSysex &_sysex;

    // void sendSingle(unsigned char &type, unsigned char &note, unsigned char &velocity);
};

}