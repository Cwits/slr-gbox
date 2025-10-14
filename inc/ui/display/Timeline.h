// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lvgl.h"
#include "defines.h"
#include "ui/display/primitives/BaseWidget.h"

#include <vector>

namespace UI {

struct Timeline : public BaseWidget {
    Timeline(BaseWidget * parent);
    ~Timeline();

    void setNudge(slr::frame_t nudge);
    slr::frame_t nudge() const { return _currentNudge; }

    void update();
    void updatePlayhead(slr::frame_t position);
    void updatePlayheadZ();

    void showLoopMarkers(bool onoff);
    void updateLoopMarkers();
    private:
    lv_style_t _font;
    
    struct line {
        lv_obj_t * _line;
        lv_point_precise_t _points[2];
    };

    bool _firstTime;
    std::vector<lv_obj_t*> _labels;
    std::vector<line> _lines;

    line _playhead;

    struct loop {
        void update(float hZoom, bool firstTime);
        void updateZ();
        void clear();
        void show(bool onoff);
        void nudge(slr::frame_t oldnudge, slr::frame_t nudge, float hzoom);

        line _loopMarkers[2];
        lv_obj_t * _fillRect;
        Timeline * _timeline = nullptr;

        private:
        struct loopHandle : public BaseWidget {
            loopHandle(BaseWidget * parent, const bool isStartHandle, Timeline * timeline);
            ~loopHandle();

            void show(bool onoff);
            void nudge(float x);
            void setY(int y);
            void updateZ();

            private:
            Timeline * _timeline = nullptr;
            lv_obj_t * _handle;
            const bool _isStartHandle;
            float _lastPosition;

            bool handleDrag(GestLib::DragGesture & drag);
        };
        loopHandle * _loopStartHandle;
        loopHandle * _loopEndHandle;

    };

    loop _loop;

    slr::frame_t _currentNudge;
    float _horizontalZoom;

};


}