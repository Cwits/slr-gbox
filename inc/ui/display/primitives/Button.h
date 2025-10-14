// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "lvgl.h"
#include <functional>
#include <string>
#include "ui/display/primitives/BaseWidget.h"

namespace UI {

class Button : public BaseWidget {
    public:
    Button(BaseWidget * parent, std::string text = "");
    ~Button();

    void setPos(lv_coord_t x, lv_coord_t y);
    void setSize(lv_coord_t x, lv_coord_t y);
    void setText(std::string text);
    void setFont(const lv_font_t *value);
    void setTextColorHex(const uint32_t value);
    const std::string text() const;
    
    void setCallback(std::function<void()> onClick);
    void setTouchDownCallback(std::function<void()> onTouchDown);
    void setTouchUpCallback(std::function<void()> onTouchUp);

    private:
    lv_obj_t * _parent;
    lv_obj_t * _btn;
    lv_obj_t * _label;
    std::function<void()> _onClick;
    std::function<void()> _onTouchDown;
    std::function<void()> _onTouchUp;

    // static void event_trampoline(lv_event_t* e);
    bool handleTouchDown(GestLib::TouchDownEvent &down) override;
    bool handleTouchUp(GestLib::TouchUpEvent &up) override;
    bool handleTap(GestLib::TapGesture & tap) override;
};

}