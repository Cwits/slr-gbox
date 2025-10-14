// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lvgl.h"
#include "ui/display/primitives/BaseWidget.h"

#include <functional>

namespace UI {

struct Label : public BaseWidget {
    Label(BaseWidget * parent, std::string text = "");
    ~Label();

    void setText(std::string text);
    std::string text() const;
    void setTextColor(const lv_color_t color);

    void setFont(const lv_font_t * font);
    void setTextPos(int x, int y);

    lv_obj_t * label() { return _label; }

    void setTapCallback(std::function<void()> onTap);
    void setHoldCallback(std::function<void()> onHold);

    private:
    lv_obj_t * _label;

    std::function<void()> _onTap;
    std::function<void()> _onHold;

    // bool handleTouchDown(GestLib::TouchDownEvent & touchDown) override;
    // bool handleTouchUp(GestLib::TouchUpEvent & touchUp) override;
    bool handleTap(GestLib::TapGesture &tap) override;
    bool handleHold(GestLib::HoldGesture &hold) override;
};

}