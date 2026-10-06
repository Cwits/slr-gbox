// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display/primitives/Checkbox.h"
#include "display/utility/layoutSizes.h"
#include "display/utility/DefaultStyles.h"

namespace UI {

Checkbox::Checkbox(BaseWidget * parent) :
    BaseWidget(parent, true)
{
    _state = false;

    setSize(Layout::CHECKBOX, Layout::CHECKBOX);

    _checked = lv_label_create(lvhost());
    lv_obj_center(_checked);
    lv_obj_set_style_text_font(_checked, &DEFAULT_FONT, 0);
    addStyle(&Style::borderless);

    tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        if(this->_state) {
            this->_state = false;
        } else {
            this->_state = true;
        }

        this->updateCheck();

        if(this->_callback) 
            this->_callback(_state);

        return true;
    });

    updateCheck();
    
    show();
}

Checkbox::~Checkbox() {
    lv_obj_delete(_checked);
}

void Checkbox::checkCallback(std::function<void(bool)> fn) {
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

}