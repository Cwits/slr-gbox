// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display/utility/DefaultColors.h"
//00rgb

namespace UI {

/*
    1.
    to use:
    0x051923 - background
    0x2a9d8f - buttons, elements,
    0x397367
    0x42858C
    0x7C898B
    0xB4ADEA

    0xE94F37 - button touch down
    0x96ACB7 or 0xD4E4BC - labels and texts

    2. https://coolors.co/19647e-28afb0-f4d35e-ee964b-564154
    0x564154 - background
    0x19647E - popups
    0xF4D35E - texts, accents - light
    0xEE964B - grid lines? - light
    0x28AFB0 - something
    0xFF9F1C
    */
namespace Colors {

const lv_color_t Red = lv_color_hex(0x00ff0000);
const lv_color_t Green = lv_color_hex(0x0000ff00);
const lv_color_t Blue = lv_color_hex(0x000000ff);
const lv_color_t Yellow = lv_color_hex(0x00ffff00);
const lv_color_t White = lv_color_hex(0x00ffffff);
const lv_color_t Black = lv_color_hex(0x00000000);
const lv_color_t Gray = lv_color_hex(0x00777777);

const lv_color_t ButtonPressed = lv_color_hex(0xE94F37);
const lv_color_t ButtonReleased = lv_color_hex(0x003559);

const lv_color_t Text = White;

const lv_color_t MuteOn = Blue;
const lv_color_t SoloOn = Yellow;
const lv_color_t Playing = Green;
const lv_color_t Paused = Gray;
const lv_color_t RecordOn = Red;
const lv_color_t LoopOn = Green;
const lv_color_t MetronomeOn = Green;

const lv_color_t GridLine = lv_color_hex(0xEE964B);
const lv_color_t LoopLine = lv_color_hex(0xF4D35E);

const lv_color_t FloatingTextRegular = White;
const lv_color_t FloatingTextWarning = Red;

const lv_color_t WorkspaceBackground = lv_color_hex(0x564154);
const lv_color_t PanelsBackground = lv_color_hex(0x564154);
const lv_color_t PopupBackground = lv_color_hex(0x19647E);
const lv_color_t DropDownBackground = lv_color_hex(0x051923);
// void initDefault() {

// }

} //namespace colors

}