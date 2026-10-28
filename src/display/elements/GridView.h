// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "display/primitives/BaseWidget.h"

#include <vector>
#include <memory>

namespace slr {
    // class TrackView;
    class ContainerItemView;
}

namespace UI {
// class TrackGui;
class GridView;
struct Timeline;
struct UIContext;

struct GridControl : public BaseWidget {
    GridControl(GridView * parent, UIContext * const uictx);
    ~GridControl();

    lv_obj_t * _lastSelectedRect;

    void pollUIUpdate() override;

    private:
    GridView * _grid;
    UIContext * const _uictx;

    bool handleTap(const GestLib::TapGesture &tap);
};

struct GridGrid : public BaseWidget { 
    GridGrid(GridView * parent, UIContext * const uictx);
    ~GridGrid();

    void pollUIUpdate() override;

    private:    
    GridView * _grid;
    UIContext * const _uictx;
};

struct GridView : public BaseWidget {
    GridView(BaseWidget * parent, UIContext * const uictx);
    ~GridView();

    const float hZoom() const { return _horizontalZoom; }
    void updateTimeline();
    void pollUIUpdate() override;

    std::unique_ptr<GridControl> _control;
    std::unique_ptr<Timeline> _timeline;
    std::unique_ptr<GridGrid> _grid;
    
    private:
    UIContext * const _uictx;
    
    double _horizontalZoom = 1.0f;

    bool handleSwipe(const GestLib::SwipeGesture & swipe);
    bool handleDrag(const GestLib::DragGesture &drag);
    bool handleZoom(const GestLib::ZoomGesture &zoom);
};

}