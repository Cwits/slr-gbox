// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

// #include "core/primitives/MidiEvent.h"
// #include <vector>

namespace PushLib {

class PushLeds;

struct PushButtonLayout {
    //use PushLeds to color all buttons.
    virtual void renderButtons(PushLeds &leds) = 0;
    
    //use PushLeds to color specific pad
    // virtual void renderButton(PushLeds &leds, const unsigned char &pad, const slr::MidiEventType &type) = 0;
};


}