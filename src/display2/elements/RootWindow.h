// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "display2/primitives/BaseWidget.h"
#include "display2/primitives/Context.h"
#include "common/defines.h"

#include <memory>

namespace Display {

struct TopPanel;
struct Grid;
struct BottomPanel;

struct RootWindow : public BaseWidget {
    RootWindow(lv_obj_t * screen);
    ~RootWindow();
    
    //Gestures
    bool handleGesture(GestLib::Gesture & gesture);
    void transferGesture(BaseWidget * target, GestLib::Gestures gesture);
    void clearHittestTarget() { _initialGestureTarget = nullptr; }
    bool cancleGesture(BaseWidget * widget);

    private:
    BaseWidget * _gestureTarget;
    BaseWidget * _initialGestureTarget;
    BaseWidget * hitTest(BaseWidget * node, int x, int y);

    Context _ctx;

    std::unique_ptr<TopPanel> _topPanel;
    std::unique_ptr<Grid> _grid;
    std::unique_ptr<BottomPanel> _bottomPanel;

    friend class Context;
};

}