// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/defaultColors.h"
//00rgb

namespace UI {

const lv_color_t RED_COLOR = lv_color_hex(0x00ff0000);
const lv_color_t GREEN_COLOR = lv_color_hex(0x0000ff00);
const lv_color_t BLUE_COLOR = lv_color_hex(0x000000ff);
const lv_color_t YELLOW_COLOR = lv_color_hex(0x00ffff00);
const lv_color_t WHITE_COLOR = lv_color_hex(0x00ffffff);
const lv_color_t BLACK_COLOR = lv_color_hex(0x00000000);
const lv_color_t GRAY_COLOR = lv_color_hex(0x00777777);

const lv_color_t BUTTON_DEFAULT_PRESSED = lv_color_hex(0x004a0a15);
const lv_color_t BUTTON_DEFAULT_COLOR = GRAY_COLOR;

const lv_color_t MUTE_ON_COLOR = BLUE_COLOR;
const lv_color_t MUTE_OFF_COLOR = BUTTON_DEFAULT_COLOR;

const lv_color_t PLAY_ON_COLOR = GREEN_COLOR;
const lv_color_t PLAY_PAUSE_COLOR = lv_color_hex(0x00111111);

const lv_color_t REC_ON_COLOR = RED_COLOR;

const lv_color_t LOOP_ON_COLOR = GREEN_COLOR;

const lv_color_t METRONOME_ON_COLOR = GREEN_COLOR;

const lv_color_t FLOATING_TEXT_REGULAR_COLOR = BLACK_COLOR;
const lv_color_t FLOATING_TEXT_WARNING_COLOR = RED_COLOR;

const lv_color_t KEYBOARD_BACKGROUND_COLOR = lv_color_hex(0x00e134eb);
}