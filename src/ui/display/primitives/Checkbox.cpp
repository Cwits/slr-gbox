// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/primitives/Checkbox.h"
#include "ui/display/layoutSizes.h"

namespace UI {

Checkbox::Checkbox(BaseWidget * parent) :
    BaseWidget(parent, true)
{
    _flags.isTap = true;
    _state = false;

    setSize(LayoutDef::CHECKBOX_SIZE, LayoutDef::CHECKBOX_SIZE);

    _checked = lv_label_create(lvhost());
    lv_obj_center(_checked);
    lv_obj_set_style_text_font(_checked, &DEFAULT_FONT, 0);
    updateCheck();
    
    show();
}

Checkbox::~Checkbox() {
    lv_obj_delete(_checked);
}

void Checkbox::setCallback(std::function<void(bool)> fn) {
    _callback = std::move(fn);
}

void Checkbox::setValue(bool checked) {
    _state = checked;
    updateCheck();
}

void Checkbox::updateCheck() {
    if(!_state) {
        lv_label_set_text(_checked, "");
    } else {
        lv_label_set_text(_checked, LV_SYMBOL_OK);
    }
}

bool Checkbox::handleTap(GestLib::TapGesture &tap) {
    if(_state) {
        _state = false;
    } else {
        _state = true;
    }

    updateCheck();

    if(_callback) 
        _callback(_state);

    return true;
}


}