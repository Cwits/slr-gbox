// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "display/primitives/BaseWidget.h"
#include "common/defines.h"

#include <vector>

namespace UI {

struct UIContext;

struct Timeline : public BaseWidget {
    Timeline(BaseWidget * parent, lv_obj_t *host, float * const zoomPtr, UIContext * uictx);
    ~Timeline();

    void setSize(lv_coord_t w, lv_coord_t h) override;
    void setPos(lv_coord_t x, lv_coord_t y) override;
    // void rebuildTimeline(float horZoom);
    void rebuildTimeline();
    
    void nudge(slr::frame_t nudge);
    inline slr::frame_t nudge() const { return _nudge; }

    void showLoopMarkers();
    void hideLoopMarkers();
    void updateLoopMarkers(float hzoom);
    void moveToFront();

    void updatePlayhead(slr::frame_t playhead);

    void pollUIUpdate() override;

    private:
    UIContext * const _uictx;
    lv_obj_t * _host;
    float * const _zoomPtr;

    slr::frame_t _nudge;

    int _width;
    int _height;
    int _x;
    int _y;

    lv_obj_t * _numberBackground;
    lv_style_t _numberFont;

    struct GridLine {
        lv_obj_t * number;
        lv_obj_t * line;
        lv_point_precise_t points[2];

        void show();
        void hide();
    };

    std::vector<GridLine> _lines;
    GridLine _playhead;

    struct LoopThings;
    struct LoopHandle : BaseWidget {
        LoopHandle(BaseWidget *parent, Timeline *tl, LoopThings *lparent);
        ~LoopHandle();

        LoopThings *_lparent;
        Timeline * _tl;
        bool handleDrag(const GestLib::DragGesture &drag);
    };

    struct LoopThings {
        ~LoopThings();
        void show();
        void hide();

        lv_obj_t * filler;

        lv_obj_t *lineStart;
        lv_point_precise_t pointsStart[2];

        lv_obj_t *lineEnd;
        lv_point_precise_t pointsEnd[2];

        std::unique_ptr<LoopHandle> handleStart;
        std::unique_ptr<LoopHandle> handleEnd;
        bool visible;
    };

    LoopThings _loop;

    uint64_t _playheadVersion;
};


}