// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/PushLib.h"
#include "push/Color.h"

#include <functional>
#include <string>
#include <vector>

namespace PushLib {

// struct PadLayout {
//     Color pad[64];
// };

class PushCore;
class PushLeds;
class PushPads;
class PushPadLayout;
class PushButtonLayout;

struct PushContext {
    void setButtonColor(ButtonColor &clr);
    void setButtonsColors(std::vector<ButtonColor> &clrs);
    void clearButtonColors();

    void setPadsLayout(PushPadLayout * layout);
    PushPadLayout const * currentPadLayout() const;
    void updatePadsColors();

    void forceRedraw();

    // void setPadLayoutUpdateCallback(std::function<void()> clb);
    // void setPadLayoutCustom(PadLayout & layout);

    // void floatingText(std::string text);

    private:
    PushCore *_core;
    PushLeds *_leds;
    PushPads *_pads;

    friend class PushCore;
};

}