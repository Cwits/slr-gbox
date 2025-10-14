// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/Timeline.h"
#include "ui/display/layoutSizes.h"
#include "ui/display/defaultStyles.h"
#include "ui/uiutility.h"

#include "snapshots/TimelineView.h"

#include "core/Events.h"
#include "defines.h"
#include "logger.h"

#include <cmath>

namespace UI {

Timeline::Timeline(BaseWidget * parent) : BaseWidget(parent, true) {
    setPos(LayoutDef::TIMELINE_X, LayoutDef::TIMELINE_Y);
    setSize(LayoutDef::TIMELINE_WIDTH, LayoutDef::TIMELINE_HEIGHT);
    lv_obj_add_style(_lvhost, &workspace, 0);
    setColor(lv_palette_main(LV_PALETTE_PINK));
    lv_obj_set_scrollbar_mode(_lvhost, LV_SCROLLBAR_MODE_OFF);

    _currentNudge = 0;
    _horizontalZoom = 1.0f;
    
    lv_style_init(&_font);
    lv_style_set_text_font(&_font, &lv_font_montserrat_16);
    lv_style_set_text_color(&_font, lv_color_hex(0xffffff));

    _firstTime = true;
    _loop._timeline = this;
    update();
    _firstTime = false;

    _flags.isDrag = true;

    show();
}

Timeline::~Timeline() {
    for(line & l : _lines) {
        lv_obj_delete(l._line);
    }
    for(lv_obj_t* o : _labels) {
        lv_obj_delete(o);
    }

    lv_obj_delete(_playhead._line);
    _loop.clear();
}

void Timeline::update() {
    //recalc zoom, lines and etc.
    int pixPerBar = UIUtility::pixelPerBar(_horizontalZoom);
    int barsOnDisplay = (LayoutDef::TIMELINE_WIDTH / pixPerBar) + 2;

    if(!_firstTime) {
        for(line &l : _lines) {
            lv_obj_delete(l._line);
        }
        _lines.clear();
        for(lv_obj_t *o : _labels) {
            lv_obj_delete(o);
        }
        _labels.clear();
    }

    //number labels
    _labels.reserve(barsOnDisplay);
    for(int i=0; i<barsOnDisplay; ++i) {
        lv_obj_t * lbl = lv_label_create(_lvhost);
        lv_obj_set_size(lbl, LayoutDef::TIMELINE_LABEL_SIZE, LayoutDef::TIMELINE_LABEL_SIZE);
        lv_obj_set_pos(lbl, pixPerBar*i, 0);
        // lv_label_set_text(lbl, std::to_string(i+1).c_str());
        lv_label_set_text_fmt(lbl, "%d", (i+1));
        lv_obj_add_style(lbl, &_font, 0);
        _labels.push_back(lbl);
    }
    
    //lines
    _lines.reserve(barsOnDisplay);
    for(int i=0; i<barsOnDisplay; ++i) {
        _lines.emplace_back();
        line &lin = _lines.back();
        lin._line = lv_line_create(parent()->lvhost());
        lin._points[0] = {pixPerBar*i, LayoutDef::TIMELINE_LINE_Y};
        lin._points[1] = {pixPerBar*i, LayoutDef::TIMELINE_LINE_HEIGHT};
        lv_line_set_points(lin._line, &lin._points[0], 2);
        
        lv_obj_add_style(lin._line, &gridLine, 0);
    }

    _playhead._line = lv_line_create(parent()->lvhost());
    _playhead._points[0] = {0, LayoutDef::TIMELINE_LINE_Y};
    _playhead._points[1] = {0, LayoutDef::TIMELINE_LINE_HEIGHT};
    lv_line_set_points(_playhead._line, &_playhead._points[0], 2);
    lv_obj_add_style(_playhead._line, &playheadStyle, 0);

    _loop.update(_horizontalZoom, _firstTime);

    slr::TimelineView & tl = slr::TimelineView::getTimelineView();
    showLoopMarkers(tl.looping());

    updatePlayheadZ();
    setNudge(_currentNudge);
}

void Timeline::updatePlayhead(slr::frame_t position) {
    slr::TimelineView & tl = slr::TimelineView::getTimelineView();
    // int framesPerBar = tl.framesPerBar();
    int pixPerBar = UIUtility::pixelPerBar(_horizontalZoom);
    float framesPerPixel = (float)pixPerBar / tl.framesPerBar();
    float res = std::round(framesPerPixel*(position-_currentNudge));

    _playhead._points[0] = {res, LayoutDef::TIMELINE_LINE_Y}; 
    _playhead._points[1] = {res, LayoutDef::TIMELINE_LINE_HEIGHT};
    lv_line_set_points(_playhead._line, &_playhead._points[0], 2);

    //не понятно...
    lv_obj_invalidate(_playhead._line);

    //если при создании playhead в points сделать +n то отрисовывается до +n а потом пропадает... странно очень
}

void Timeline::updatePlayheadZ() {
    lv_obj_move_to_index(_playhead._line, -1);
    lv_obj_move_to_index(lvhost(), -1);
    _loop.updateZ();
}

void Timeline::setNudge(slr::frame_t nudge) {
    slr::frame_t oldNudge = _currentNudge;
    _currentNudge = nudge;
    
    //let's leave creating or deleting points to zoom update
    slr::TimelineView & tl = slr::TimelineView::getTimelineView();
    int pixPerBar = UIUtility::pixelPerBar(_horizontalZoom);
    float pixMoved = ( (float)nudge / tl.framesPerBar() ) - ( (int)nudge/tl.framesPerBar() );

    int startBar = (nudge/tl.framesPerBar()) + 1;
    float wtf = pixMoved * pixPerBar;
    int wtf2 = std::round(wtf);

    std::size_t size = _labels.size();
    for(std::size_t i=0; i<size; ++i, ++startBar) {
        int x = (pixPerBar*i) - wtf2;

        lv_obj_t * label = _labels.at(i);
        lv_obj_set_pos(label, x, 0);
        lv_label_set_text_fmt(label, "%d", startBar);

        line & l = _lines.at(i);
        l._points[0] = {x, LayoutDef::TIMELINE_LINE_Y};
        l._points[1] = {x, LayoutDef::TIMELINE_LINE_HEIGHT};
        lv_line_set_points(l._line, &l._points[0], 2);
        
        lv_obj_invalidate(label);
        lv_obj_invalidate(l._line);
    }

    //update loop markers?
    _loop.nudge(oldNudge, nudge, _horizontalZoom);
}

void Timeline::showLoopMarkers(bool onoff) {
    _loop.show(onoff);
}

void Timeline::updateLoopMarkers() {
    _loop.update(_horizontalZoom, false);
}

void Timeline::loop::update(float hZoom, bool firstTime) {


    if(!firstTime) {
        clear();
    }
    slr::TimelineView & tl = slr::TimelineView::getTimelineView();
    //loop markers
    slr::frame_t loopStart = tl.loopStartFrame();
    slr::frame_t loopEnd = tl.loopEndFrame();

    float resStart = UIUtility::frameToPixel(loopStart, _timeline->_horizontalZoom);
    float resEnd = UIUtility::frameToPixel(loopEnd, _timeline->_horizontalZoom);

    _loopMarkers[0]._line = lv_line_create(_timeline->parent()->lvhost());
    _loopMarkers[1]._line = lv_line_create(_timeline->parent()->lvhost());

    _loopMarkers[0]._points[0] = {resStart, LayoutDef::TIMELINE_LINE_Y};
    _loopMarkers[0]._points[1] = {resStart, LayoutDef::TIMELINE_LINE_HEIGHT};
    
    _loopMarkers[1]._points[0] = {resEnd, LayoutDef::TIMELINE_LINE_Y};
    _loopMarkers[1]._points[1] = {resEnd, LayoutDef::TIMELINE_LINE_HEIGHT};
    
    lv_line_set_points(_loopMarkers[0]._line, &_loopMarkers[0]._points[0], 2);
    lv_line_set_points(_loopMarkers[1]._line, &_loopMarkers[1]._points[0], 2);
    
    lv_obj_add_style(_loopMarkers[0]._line, &loopMarkersStyle, 0);
    lv_obj_add_style(_loopMarkers[1]._line, &loopMarkersStyle, 0);

    //fill rect
    _fillRect = lv_obj_create(_timeline->parent()->lvhost());
    lv_obj_set_size(_fillRect, resEnd-resStart, LayoutDef::TIMELINE_LINE_HEIGHT);
    lv_obj_set_pos(_fillRect, resStart, LayoutDef::TIMELINE_LINE_Y);
    lv_obj_add_style(_fillRect, &loopFillStyle, 0);

    lv_obj_invalidate(_loopMarkers[0]._line);
    lv_obj_invalidate(_loopMarkers[1]._line);
    lv_obj_invalidate(_fillRect);

    //loop handles
    _loopStartHandle = new loopHandle(_timeline->parent(), true, _timeline);
    _loopStartHandle->nudge(resStart);
    _loopStartHandle->setY(LayoutDef::TIMELINE_LINE_Y);
    _loopEndHandle = new loopHandle(_timeline->parent(), false, _timeline);
    _loopEndHandle->nudge(resEnd-LayoutDef::TIMELINE_LOOP_HANDLE_W);
    _loopEndHandle->setY(LayoutDef::TIMELINE_LINE_HEIGHT-LayoutDef::TIMELINE_LOOP_HANDLE_H);
}

void Timeline::loop::clear() {
    lv_obj_delete(_loopMarkers[0]._line);
    lv_obj_delete(_loopMarkers[1]._line);
    lv_obj_delete(_fillRect);
    delete _loopStartHandle;
    delete _loopEndHandle;
}

void Timeline::loop::show(bool onoff) {
    if(onoff) {
        lv_obj_clear_flag(_loopMarkers[0]._line, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(_loopMarkers[1]._line, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(_fillRect, LV_OBJ_FLAG_HIDDEN);
        
    } else {
        lv_obj_add_flag(_loopMarkers[0]._line, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_loopMarkers[1]._line, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_fillRect, LV_OBJ_FLAG_HIDDEN);
    }
    
    // LOG_WARN("points start %f, end %f", _loopMarkers[0]._points[0].x, _loopMarkers[1]._points[0].x);
    _loopStartHandle->show(onoff);
    _loopEndHandle->show(onoff);
    
    //somehow lvgl doesn't update position when objects not visible
    if(onoff)
        nudge(0, _timeline->_currentNudge, _timeline->_horizontalZoom); 
}

void Timeline::loop::updateZ() { 
    lv_obj_move_to_index(_loopMarkers[0]._line, -1);
    lv_obj_move_to_index(_loopMarkers[1]._line, -1);
    lv_obj_move_to_index(_fillRect, -1);
    _loopStartHandle->updateZ();
    _loopEndHandle->updateZ();
}

void Timeline::loop::nudge(slr::frame_t oldnudge, slr::frame_t nudge, float hzoom) {
    slr::TimelineView & tl = slr::TimelineView::getTimelineView();

    slr::frame_t loopStart = tl.loopStartFrame();
    slr::frame_t loopEnd = tl.loopEndFrame();

    int pixPerBar = UIUtility::pixelPerBar(hzoom);
    float framesPerPixel = (float)pixPerBar / tl.framesPerBar();
    float resStart = std::round(framesPerPixel*loopStart);
    float resEnd = std::round(framesPerPixel*loopEnd);

    slr::frame_t test = UIUtility::pixelToFrame(resEnd, hzoom);
    // LOG_WARN("test: loopEnd %lu, pix: %f, res: %lu", loopEnd, resEnd, test);

    float pixMoved = ( (float)nudge / tl.framesPerBar() );

    float wtf = pixMoved * pixPerBar;
    int wtf2 = std::round(wtf);

    resStart -= wtf2;
    resEnd -= wtf2;

    _loopMarkers[0]._points[0] = {resStart, LayoutDef::TIMELINE_LINE_Y};
    _loopMarkers[0]._points[1] = {resStart, LayoutDef::TIMELINE_LINE_HEIGHT};
    
    _loopMarkers[1]._points[0] = {resEnd, LayoutDef::TIMELINE_LINE_Y};
    _loopMarkers[1]._points[1] = {resEnd, LayoutDef::TIMELINE_LINE_HEIGHT};
    
    // LOG_WARN("points start %f, end %f", _loopMarkers[0]._points[0].x, _loopMarkers[1]._points[0].x);
    lv_line_set_points(_loopMarkers[0]._line, &_loopMarkers[0]._points[0], 2);
    lv_line_set_points(_loopMarkers[1]._line, &_loopMarkers[1]._points[0], 2);

    lv_obj_set_pos(_fillRect, resStart, LayoutDef::TIMELINE_LINE_Y);
    lv_obj_set_size(_fillRect, resEnd-resStart, LayoutDef::TIMELINE_LINE_HEIGHT);

    lv_obj_invalidate(_loopMarkers[0]._line);
    lv_obj_invalidate(_loopMarkers[1]._line);
    lv_obj_invalidate(_fillRect);

    _loopStartHandle->nudge(resStart);
    _loopEndHandle->nudge(resEnd-LayoutDef::TIMELINE_LOOP_HANDLE_W);
}

Timeline::loop::loopHandle::loopHandle(BaseWidget * parent, const bool isStartHandle, Timeline * timeline) :
    BaseWidget(parent),
    _isStartHandle(isStartHandle), 
    _timeline(timeline)
{
    _flags.isDrag = true;
    _handle = lv_obj_create(parent->lvhost());
    _lvhost = _handle;
    lv_obj_set_size(_handle, LayoutDef::TIMELINE_LOOP_HANDLE_W, LayoutDef::TIMELINE_LOOP_HANDLE_H);
    lv_obj_add_style(_handle, &loopHandleStyle, 0);
}

Timeline::loop::loopHandle::~loopHandle() {
    lv_obj_delete(_handle);
}

bool Timeline::loop::loopHandle::handleDrag(GestLib::DragGesture & drag) {
    // LOG_WARN("Here");
    // static uint8_t inc = 0;
    // inc++;
    // //reduce ammount of actions
    // if(inc < 5) return false;
    // inc = 0;

    switch(drag.state) {
        case(GestLib::GestureState::Start): {

        } break;
        case(GestLib::GestureState::Move): {
            //TODO: need to snap to grid...
            int cx = drag.x - LayoutDef::GRID_X;
            lv_obj_set_x(_handle, cx);
            if(_isStartHandle) {
                _timeline->_loop._loopMarkers[0]._points[0].x = cx;
                _timeline->_loop._loopMarkers[0]._points[1].x = cx;
                lv_line_set_points(_timeline->_loop._loopMarkers[0]._line, 
                                    &_timeline->_loop._loopMarkers[0]._points[0], 2);
                
                int origwidth = lv_obj_get_width(_timeline->_loop._fillRect);
                int origx = lv_obj_get_x(_timeline->_loop._fillRect);
                int diff = origx - cx;
                lv_obj_set_x(_timeline->_loop._fillRect, cx);
                lv_obj_set_width(_timeline->_loop._fillRect, origwidth + diff);
            } else {
                _timeline->_loop._loopMarkers[1]._points[0].x = cx+LayoutDef::TIMELINE_LOOP_HANDLE_W;
                _timeline->_loop._loopMarkers[1]._points[1].x = cx+LayoutDef::TIMELINE_LOOP_HANDLE_W;
                lv_line_set_points(_timeline->_loop._loopMarkers[1]._line, 
                                    &_timeline->_loop._loopMarkers[1]._points[0], 2);
                
                int origx = lv_obj_get_x(_timeline->_loop._fillRect);
                lv_obj_set_width(_timeline->_loop._fillRect, cx+LayoutDef::TIMELINE_LOOP_HANDLE_W-origx);
            }
        } break;
        case(GestLib::GestureState::End): {
            int cx = lv_obj_get_x(_handle);

            if(_isStartHandle) {
                //loop start event
            } else {
                //loop end event
                cx += LayoutDef::TIMELINE_LOOP_HANDLE_W;
            }

            slr::frame_t res = UIUtility::pixelToFrame(cx, _timeline->_horizontalZoom);
            LOG_WARN("result: %lu", res);
            //TODO: Snap to grid
            slr::TimelineView & tl = slr::TimelineView::getTimelineView();
            if(_isStartHandle) {
                slr::Events::LoopPosition e = {
                    .start = res,
                    .end = tl.loopEndFrame()
                };
                slr::EmitEvent(e);
            } else {
                slr::Events::LoopPosition e = {
                    .start = tl.loopStartFrame(),
                    .end = res
                };
                slr::EmitEvent(e);
            }
            
            //slr::EmitEvet()
        } break;
    }
    return true;
}

void Timeline::loop::loopHandle::setY(int y) {
    lv_obj_set_y(_handle, y);
}

void Timeline::loop::loopHandle::show(bool onoff) {
    if(onoff) {
        lv_obj_clear_flag(_handle, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(_handle, LV_OBJ_FLAG_HIDDEN);
    }
}

void Timeline::loop::loopHandle::nudge(float x) {
    lv_obj_set_x(_handle, static_cast<int>(x));
    _lastPosition = x;
    lv_obj_invalidate(_handle);
}

void Timeline::loop::loopHandle::updateZ() {
    lv_obj_move_to_index(_handle, -1);
    lv_obj_invalidate(_handle);
}


}