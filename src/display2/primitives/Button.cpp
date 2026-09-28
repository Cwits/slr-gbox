// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display2/primitives/Button.h"
#include "display2/utility/DefaultStyle.h"
#include "display2/utility/DefaultColors.h"

namespace Display {

Button::Button(BaseWidget * parent, std::string text) : BaseWidget(parent, false) {
    _btn = lv_btn_create(parent->lvhost());
    _lvhost = _btn;
    _hasHost = true;
    _isRoot = true;
    _disabled = false;
    
    _label = lv_label_create(_btn);
    lv_obj_add_style(_btn, &DefaultStyle::ButtonStyle, 0);
    lv_label_set_text(_label, text.c_str());
    lv_obj_center(_label);
    // lv_obj_set_style_bg_color(_btn, DefaultStyle::Button, LV_PART_MAIN);
    
    touchDownCallback([btn = _btn](const GestLib::TouchDownEvent &td) {
        lv_obj_set_style_bg_color(btn, DefaultColors::PressedButton, LV_PART_MAIN);
    });
    touchUpCallback([btn = _btn](const GestLib::TouchUpEvent &tu) {
        lv_obj_set_style_bg_color(btn, DefaultColors::Button, LV_PART_MAIN);
    });

    setFont(DefaultStyle::Font);
}

Button::~Button() {
    lv_obj_delete(_label);
    lv_obj_delete(_btn);
}

void Button::setPos(lv_coord_t x, lv_coord_t y) {
    lv_obj_set_pos(_btn, x, y);
}

void Button::setSize(lv_coord_t x, lv_coord_t y) {
    lv_obj_set_size(_btn, x, y);
}

void Button::setText(std::string text) {
    lv_label_set_text(_label, text.c_str());
}

void Button::setFont(const lv_font_t *value) {
    lv_obj_set_style_text_font(_label, value, 0);
}

void Button::setTextColorHex(const uint32_t value) {
    lv_obj_set_style_text_color(_label, lv_color_hex(value), 0);
}

void Button::setTextPos(lv_coord_t x, lv_coord_t y) {
    lv_obj_set_pos(_label, x, y);
}

const std::string Button::text() const {
    const std::string ret = std::string(lv_label_get_text(_label));
    return ret;
}

}