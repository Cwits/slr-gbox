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

    void setLedColor(unsigned char lednum, LedAnimation anim, unsigned char color);
    void setLedColors(std::vector<unsigned char> &lednums, LedAnimation anim, unsigned char clr);
    void clearAllLeds();

    private:
    PushMidi &_midi;
    PushSysex &_sysex;
};

}