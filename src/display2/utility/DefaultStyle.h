// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lvgl.h"

namespace Display {

namespace DefaultStyle {

extern lv_style_t Borderless;
extern lv_style_t ButtonStyle;
extern lv_style_t Workspace;
// extern lv_style_t ButtonLabelStyle;
extern lv_style_t LabelStyle;

/* Grid */
extern lv_style_t gridLine;
extern lv_style_t playheadStyle;

extern lv_style_t loopMarkersStyle;
extern lv_style_t loopFillStyle;
extern lv_style_t loopHandleStyle;
/* Browser */
extern lv_style_t browserElementStyle;

extern const lv_font_t * Font;

void initDefaultStyle();

}

}