// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "display2/primitives/BaseWidget.h"

#include <functional>
#include <string>

namespace Display {

class Button : public BaseWidget {
    public:  
    Button(BaseWidget * parent, std::string text = "");
    ~Button();

    void setPos(lv_coord_t x, lv_coord_t y);
    void setSize(lv_coord_t x, lv_coord_t y);

    void setText(std::string text);
    void setFont(const lv_font_t *value);
    void setTextColorHex(const uint32_t value);
    void setTextPos(lv_coord_t x, lv_coord_t y);
    const std::string text() const;
    
    void disable() { _disabled = true; }
    void enable() { _disabled = false; }
    bool isDisabled() const { return _disabled; }

    private:
    bool _disabled;

    lv_obj_t * _parent;
    lv_obj_t * _btn;
    lv_obj_t * _label;
};

}