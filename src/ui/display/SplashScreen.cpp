// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/SplashScreen.h"

namespace UI {

SplashScreen::SplashScreen(lv_obj_t * parent) : _parent(parent) {
    _host = lv_obj_create(nullptr);
    lv_obj_set_size(_host, 1920, 1080);
    lv_obj_set_pos(_host, 0, 0);
    lv_obj_set_style_bg_color(_host, lv_color_hex(0x000000), LV_PART_MAIN);
    

    _logo = lv_label_create(_host);
    lv_label_set_text(_logo, "Sample.Loop.Repeat\0");
    lv_obj_set_style_text_color(_logo, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(_logo, &lv_font_montserrat_40, 0);
    lv_obj_center(_logo);
}

SplashScreen::~SplashScreen() {
    lv_obj_delete(_logo);
    lv_obj_delete(_host);
}

}