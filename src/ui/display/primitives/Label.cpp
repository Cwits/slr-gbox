// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/primitives/Label.h"
#include "logger.h"

namespace UI {

Label::Label(BaseWidget * parent, std::string text) :
    BaseWidget(parent, true)
{
    _label = lv_label_create(_lvhost);
    lv_label_set_text(_label, text.c_str());
    lv_obj_set_style_bg_opa(_lvhost, LV_OPA_TRANSP, 0);
    lv_obj_set_scrollbar_mode(_lvhost, LV_SCROLLBAR_MODE_OFF);

    show();
}

Label::~Label() {
    lv_obj_delete(_label);
}

void Label::setText(std::string text) {
    lv_label_set_text(_label, text.c_str());
}

std::string Label::text() const {
    return std::string(lv_label_get_text(_label));
}

void Label::setTextPos(int x, int y) {
    lv_obj_set_pos(_label, x, y);
}

void Label::setTextColor(const lv_color_t color) {
    lv_obj_set_style_text_color(_label, color, 0);
}

void Label::setFont(const lv_font_t * font) {
    lv_obj_set_style_text_font(_label, font, 0);
}

void Label::setTapCallback(std::function<void()> onTap) {
    _onTap = onTap;
    _flags.isTap = true;
}

void Label::setHoldCallback(std::function<void()> onHold) {
    _onHold = onHold;
    _flags.isHold = true;
}

bool Label::handleTap(GestLib::TapGesture &tap) {
    if(_onTap) {
        _onTap();
        return true;
    }
    return false;
}

bool Label::handleHold(GestLib::HoldGesture &hold) {
    if(_onHold) {
        _onHold();
        return true;
    }
    return false;
}

// bool Label::handleTouchDown(GestLib::TouchDownEvent & touchDown) {
//     // lv_obj_set_style_text_color(_label, lv_palette_main(LV_PALETTE_PINK), 0);
//     return true;
// }

// bool Label::handleTouchUp(GestLib::TouchUpEvent & touchUp) {
//     lv_obj_set_style_text_color(_label, lv_color_hex(0x000000), 0);
//     return true;
// }



}