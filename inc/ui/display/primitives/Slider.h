// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/display/primitives/BaseWidget.h"

#include <functional>

namespace UI {

class Label;

struct Slider : public BaseWidget {
    Slider(BaseWidget * parent);
    ~Slider();

    void onChangeCallback(std::function<void(const float)> onChange);
    void setCap(const float value);
    const float getCap();

    private:
    lv_obj_t * _lblMax;
    lv_obj_t * _lblMin;
    lv_obj_t * _lblZero;

    lv_obj_t * _sliderCap;
    std::function<void(const float)> _onChange;

    bool handleDrag(GestLib::DragGesture &drag) override;
};

}