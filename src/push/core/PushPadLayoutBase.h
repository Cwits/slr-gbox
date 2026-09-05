// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/primitives/MidiEvent.h"
// #include <vector>

namespace PushLib {

class PushLeds;

struct PushPadLayout {
    //convert Pad number to midi note
    virtual const unsigned char getNote(const unsigned char &pad) = 0;

    //use PushLeds to color all 64 pads.
    virtual void renderPads(PushLeds &leds) = 0;
    
    //use PushLeds to color specific pad
    virtual void renderPad(PushLeds &leds, const unsigned char &pad, const slr::MidiEventType &type) = 0;
};


}