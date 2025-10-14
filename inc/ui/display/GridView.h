// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lvgl.h"
#include "ui/display/primitives/BaseWidget.h"
#include "ui/display/primitives/View.h"
#include "ui/display/Timeline.h"

#include <vector>
#include <memory>

namespace slr {
    class TrackView;
    class ContainerItemView;
}

namespace UI {
// class TrackGui;
class GridView;

struct GridControl : public BaseWidget {
    GridControl(GridView * parent, UIContext * const uictx);
    ~GridControl();

    lv_obj_t * _lastSelectedRect;
    private:
    GridView * _grid;
    UIContext * const _uictx;

    bool handleTap(GestLib::TapGesture &tap) override;
};

struct GridGrid : public BaseWidget { 
    GridGrid(GridView * parent, UIContext * const uictx);
    ~GridGrid();

    private:    
    GridView * _grid;
    UIContext * const _uictx;

    bool handleDrag(GestLib::DragGesture & drag) override;

};

struct GridView : public View {
    GridView(BaseWidget * parent, UIContext * const uictx);
    ~GridView();

    const float hZoom() const { return _horizontalZoom; }
    void update() override {}

    GridControl * _control;
    GridGrid * _grid;
    Timeline * _timeline;
    
    private:
    float _horizontalZoom = 1.0f;

    bool handleSwipe(GestLib::SwipeGesture & swipe) override;
};

}