// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "lvgl.h"

namespace Display {

namespace DefaultColors {

extern lv_color_t Red;
extern lv_color_t Blue;
extern lv_color_t Green;

extern lv_color_t Background;
extern lv_color_t Button;
extern lv_color_t PressedButton;
extern lv_color_t Panels;
extern lv_color_t TextColor;

void initDefaultColors();

}

}