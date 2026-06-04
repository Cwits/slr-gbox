// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/display/primitives/Popup.h"
#include <memory>
#include <chrono>

namespace UI {

struct UIContext;
struct Button;
struct Label;

struct DragViewSelector : public Popup {
    DragViewSelector(BaseWidget *parent, UIContext * const uictx);
    ~DragViewSelector();

    void update();
    void reset();

    private:
    int _lastDragX;
    int _lastDragY;
    
    lv_timer_t * _timDrag;

    lv_obj_t * _gridZoneRect;
    lv_obj_t * _unitZoneRect;

    std::unique_ptr<Label> _lblName;
    std::unique_ptr<Label> _lblGrid;
    std::unique_ptr<Label> _lblUnit;

    static void timerClb(lv_timer_t * timer);
    bool handleDrag(GestLib::DragGesture & drag) override;
};

}