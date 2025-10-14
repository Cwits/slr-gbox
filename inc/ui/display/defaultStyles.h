// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lvgl.h"

namespace UI {
    
/* General */
extern lv_style_t workspace;
extern lv_style_t buttonDefaultStyle;

/* Grid */
extern lv_style_t gridLine;
extern lv_style_t playheadStyle;

extern lv_style_t loopMarkersStyle;
extern lv_style_t loopFillStyle;
extern lv_style_t loopHandleStyle;
/* Browser */
extern lv_style_t browserElementStyle;

void initDefaultStyles();

}