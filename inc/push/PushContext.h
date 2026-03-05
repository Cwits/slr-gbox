// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/PushLib.h"
#include "push/Color.h"

#include <functional>
#include <string>

namespace PushLib {

struct PadLayout {
    Color pad[64];
};

class PushCore;
class PushLeds;
class PushPads;
class PushPadLayout;

struct PushContext {
    // void setButtonColor(Button btn, Color clr);
    //void updateButtons(std::unordered_map<Button, color> &map);

    // void setPadsNotes();
    // void setPadsColors(unsigned char root, unsigned char inScale, unsigned char offScale, unsigned char pressed);
    // void setPadsChromatic(const bool chromatic);
    // const bool isPadsChromatic();
    // void setPadsScale(const Scales scale);
    // void setPadsLayout(const LayoutStyle layout);

    // void setPadsCustom();
    void setPadsLayout(PushPadLayout * layout);
    PushPadLayout const * currentPadLayout() const;
    void updatePadsColors();

    // void setPadLayoutUpdateCallback(std::function<void()> clb);
    // void setPadLayoutCustom(PadLayout & layout);

    // void floatingText(std::string text);

    private:
    PushLeds *_leds;
    PushPads *_pads;

    friend class PushCore;
};

}