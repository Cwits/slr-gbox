// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/primitives/Button.h"
#include "ui/display/layoutSizes.h"
#include "ui/display/defaultStyles.h"
#include "ui/display/defaultColors.h"

namespace UI {

Button::Button(BaseWidget * parent, std::string text) : BaseWidget(parent, false) {
    _btn = lv_btn_create(parent->lvhost());
    _lvhost = _btn;
    _hasHost = true;
    _isRoot = true;
    
    _flags.isTap = true;
    
    _label = lv_label_create(_btn);
    lv_label_set_text(_label, text.c_str());
    lv_obj_center(_label);
    lv_obj_add_style(_btn, &buttonDefaultStyle, 0);
    
    setTouchDownCallback([this]() {
        this->setColor(BUTTON_DEFAULT_PRESSED);
    });
    setTouchUpCallback([this]() {
        this->setColor(BUTTON_DEFAULT_COLOR);
    });
    // lv_obj_add_event_cb(_btn, &Button::event_trampoline, LV_EVENT_CLICKED, this);
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
    
void Button::setCallback(std::function<void()> onClick) {
    _onClick = onClick;
}

void Button::setTouchDownCallback(std::function<void()> onTouchDown) {
    _onTouchDown = onTouchDown;
    _flags.isTouchDown = true;
}

void Button::setTouchUpCallback(std::function<void()> onTouchUp) {
    _onTouchUp = onTouchUp;
    _flags.isTouchUp = true;
}

    
// void Button::event_trampoline(lv_event_t* e) {
//     Button* self = static_cast<Button*>(lv_event_get_user_data(e));

//     if (self && self->_onClick) {
//         self->_onClick();
//     }
// }

bool Button::handleTap(GestLib::TapGesture & tap) {
    if(_onClick) {
        _onClick();
        return true;
    }
    return false;
}

bool Button::handleTouchDown(GestLib::TouchDownEvent &down) {
    if(_onTouchDown) {
        _onTouchDown();
        return true;
    }

    return false;
}

bool Button::handleTouchUp(GestLib::TouchUpEvent &up) {
    if(_onTouchUp) {
        _onTouchUp();
        return true;
    }

    return false;
}

const std::string Button::text() const {
    const std::string ret = std::string(lv_label_get_text(_label));
    return ret;
}

}