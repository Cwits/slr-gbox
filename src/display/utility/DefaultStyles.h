// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lvgl.h"

namespace UI {

namespace Style {
/* General */
extern lv_style_t borderless;
extern lv_style_t workspace;
extern lv_style_t buttonDefaultStyle;
extern lv_style_t Label;
extern lv_style_t GridControl;
extern lv_style_t GridFile;

extern lv_style_t Text;
/* Grid */
extern lv_style_t gridLine;
extern lv_style_t playheadStyle;

extern lv_style_t loopMarkersStyle;
extern lv_style_t loopFillStyle;
extern lv_style_t loopHandleStyle;
/* Browser */
extern lv_style_t browserElementStyle;

extern lv_style_t Panels;
extern lv_style_t PopupDefault;
extern lv_style_t DropDown;


void initDefaultStyles();
}

}