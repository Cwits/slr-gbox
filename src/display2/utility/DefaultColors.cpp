// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "display2/utility/DefaultColors.h"

namespace Display {

namespace DefaultColors {

/*
    to use:
    0x051923 - background
    0x2a9d8f - buttons, elements,
    0x397367
    0x42858C
    0x7C898B
    0xB4ADEA

    0xE94F37 - button touch down
    0x96ACB7 or 0xD4E4BC - labels and texts
*/

lv_color_t Red;
lv_color_t Blue;
lv_color_t Green;

lv_color_t Background;
lv_color_t Button;
lv_color_t PressedButton;
lv_color_t TextColor;
lv_color_t Panels;

void initDefaultColors() {
    
    Red = lv_color_hex(0xff0000);
    Blue = lv_color_hex(0x00ff00);
    Green = lv_color_hex(0x0000ff);
    Background = lv_color_hex(0x051923);
    Button = lv_color_hex(0x2a9d8f);
    PressedButton = lv_color_hex(0xE94F37);
    TextColor = lv_color_hex(0xD4E4BC);
    Panels = lv_color_hex(0x7C898B);
    
}

}

}