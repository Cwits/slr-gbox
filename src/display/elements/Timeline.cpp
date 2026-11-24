// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display/elements/Timeline.h"

#include "core/actions/Actions.h"

#include "display/utility/DefaultStyles.h"
#include "display/utility/DefaultColors.h"
#include "display/utility/layoutSizes.h"

#include "snapshots/TimelineView.h"

#include "common/uiutility.h"
#include "common/Math.h"
#include "common/logger.h"

#include <limits>
#include <cassert>

namespace UI {

constexpr float MIN_LINE_SPACING = 48.0f;

Timeline::Timeline(BaseWidget * parent, lv_obj_t *host, double * const zoomPtr, UIContext * uictx) :
    BaseWidget(parent, false),
    _uictx(uictx),
    _host(host),
    _zoomPtr(zoomPtr)
{
    _numberBackground = lv_obj_create(_host);
    lv_obj_add_style(_numberBackground, &Style::borderless, 0);
    lv_obj_set_style_bg_color(_numberBackground, Colors::Gray, 0);
    lv_obj_set_pos(_numberBackground, 0, 0);
    lv_obj_set_scrollbar_mode(_numberBackground, LV_SCROLLBAR_MODE_OFF);
    

    lv_style_init(&_numberFont);
    lv_style_set_text_font(&_numberFont, &lv_font_montserrat_28);
    lv_style_set_text_color(&_numberFont, Colors::White);

    _loop.filler = lv_obj_create(_host);
    _loop.lineStart = lv_line_create(_host);
    _loop.lineEnd = lv_line_create(_host);
    _loop.handleStart = std::make_unique<Timeline::LoopHandle>(parent, this, &_loop);
    _loop.handleEnd = std::make_unique<Timeline::LoopHandle>(parent, this, &_loop);

    lv_obj_add_style(_loop.filler, &Style::loopFillStyle, 0);
    lv_obj_add_style(_loop.lineStart, &Style::loopMarkersStyle, 0);
    lv_obj_add_style(_loop.lineEnd, &Style::loopMarkersStyle, 0);
    
    lv_obj_add_style(_loop.handleStart->lvhost(), &Style::loopHandleStyle, 0);
    lv_obj_add_style(_loop.handleEnd->lvhost(), &Style::loopHandleStyle, 0);
    lv_obj_set_size(_loop.handleStart->lvhost(), Layout::TIMELINE_LOOP_HANDLE_W, Layout::TIMELINE_LOOP_HANDLE_H);
    lv_obj_set_size(_loop.handleEnd->lvhost(), Layout::TIMELINE_LOOP_HANDLE_W, Layout::TIMELINE_LOOP_HANDLE_H);

    _playhead.line = lv_line_create(_host);
    lv_obj_add_style(_playhead.line, &Style::playheadStyle, 0);

    _loop.hide();

    _nudge = 0;

    show();
    _playheadVersion = 0;
}

Timeline::~Timeline() {
    lv_obj_delete(_numberBackground);
    // for(std::size_t i=0; i<_lines.size(); ++i) {
    //     lv_obj_delete(_lines[i].number);
    //     lv_obj_delete(_lines[i].line);
    // }
}

void Timeline::setSize(lv_coord_t w, lv_coord_t h) {
    // BaseWidget::setSize(w, h);
    lv_obj_set_size(_numberBackground, w, 30);
    _width = static_cast<int>(w);
    _height = static_cast<int>(h);
    updatePlayhead(0);
}

void Timeline::setPos(lv_coord_t x, lv_coord_t y) {
    // BaseWidget::setPos(x, y);
    lv_obj_set_pos(_numberBackground, x, y);
    _x = static_cast<int>(x);
    _y = static_cast<int>(y);
    updatePlayhead(0);
}

void Timeline::pollUIUpdate() {
    slr::TimelineView &tl = slr::TimelineView::getTimelineView();
    uint64_t ver = tl.playheadVersion();
    if(ver != _playheadVersion) {
        updatePlayhead(tl.elapsed());
        _playheadVersion = ver;
    }
}

void Timeline::nudge(slr::frame_t nudge) {
    _nudge = nudge;
}

std::pair<int, int> Timeline::positionsWithinGrid(int x) const {
    //assume that x position is within timeline width
    int xpos = 0;
    int width = 0;

    float pixelPerBar = UIUtility::pixelPerBar(*_zoomPtr);
    // nudge...
    
    // int gridVal = _gridSize == 0 ? (_subdivisions == 1 ? _barStep : _subdivisions) : _gridSize;
    int gridVal = _subdivisions == 1 ? _barStep : _subdivisions;
    float pixelPerSubdiv = pixelPerBar / gridVal;
    float kindaBar = (float)x / pixelPerBar;
    
    slr::TimelineView &tl = slr::TimelineView::getTimelineView();
    double nudgeBars = static_cast<double>(_nudge) / tl.framesPerBar();
    int firstIndex = static_cast<int>(Math::floor(nudgeBars * _subdivisions));
    float leftover = /*Math::round*/(pixelPerSubdiv * ((nudgeBars*_subdivisions) - (float)firstIndex));

    
    float subdiv = leftover == 0 ? 0 : pixelPerSubdiv - leftover;//Math::floor(kindaBar) * pixelPerBar + (pixelPerSubdiv - leftover);
    while(subdiv+pixelPerSubdiv < x) subdiv += pixelPerSubdiv;

    // xpos = !_nudge ? Math::floor(subdiv) : Math::floor(subdiv) - leftover + pixelPerSubdiv;
    xpos = Math::round(subdiv);
    width = pixelPerSubdiv;

    // if(_gridSize == 0) {
    //     //automatic one - use subdivisions and barStep

    // } else {
    //     //fixed grid, have to find value independently of how close or far we are?
    // }

    return std::make_pair(xpos, width);
}

slr::frame_t Timeline::positionToFrames(int x) const {
    slr::frame_t ret = 0;

    float pixelPerBar = UIUtility::pixelPerBar(*_zoomPtr);
    float barsOnDisplay = (float)_width / pixelPerBar;
    
    const slr::TimelineView &tl = slr::TimelineView::getTimelineView();
    float framesOnScreen = barsOnDisplay * (float)tl.framesPerBar();

    float coef = (float)x / (float)_width;
    ret = Math::round(framesOnScreen * coef);
    ret += _nudge;

    return ret;
}

int Timeline::absoluteFramesToPosition(slr::frame_t f) const {
    f -= _nudge;
    return UIUtility::frameToPixel(f, *_zoomPtr);
}

void Timeline::rebuildTimeline() {
    if(std::isinf(*_zoomPtr)) {
        assert(false || "Can't draw with inf zoom");
        return;
    }

    float pixelPerBar = UIUtility::pixelPerBar(*_zoomPtr);
    int barsOnDisplay = (_width / pixelPerBar) + 2; 
    slr::TimelineView &tl = slr::TimelineView::getTimelineView();
    const slr::BarSize &barSize = tl.getBarSize();

    int divider = (barSize._numerator % 2 == 0) ? 2 :
                    (barSize._numerator % 3 == 0) ? 3 :
                    (barSize._numerator % 5 == 0) ? 5 : 
                    (barSize._numerator % 7 == 0) ? 7 : 1;

    _subdivisions = 1;
    _barStep = 1;
    bool mulled = false;

    if(_gridSize == 0) {
        while(pixelPerBar / (_subdivisions * (mulled ? (_isTriplets ? 3 : 2): divider)) >= MIN_LINE_SPACING) {
            // if(mulled) { 
            //     if(divider == 2) subdivisions *= divider;
            //     else subdivisions += divider;
            // } else {
            //     subdivisions *= divider;
            //     mulled = true;
            // }
            _subdivisions *= (mulled ? (_isTriplets ? 3 : 2) : divider);
            mulled = true;
        }

        while(pixelPerBar * _barStep < MIN_LINE_SPACING) {
            _barStep *= 2;
        }
    } else {
        while(pixelPerBar / (_subdivisions * (mulled ? (_isTriplets ? 3 : 2) : divider)) >= MIN_LINE_SPACING) {
            // if(mulled) { 
            //     if(divider == 2) subdivisions *= divider;
            //     else subdivisions += divider;
            // } else {
            //     subdivisions *= divider;
            //     mulled = true;
            // }
            _subdivisions *= (mulled ? (_isTriplets ? 3 : 2) : divider);
            mulled = true;
            if(_subdivisions >= _gridSize) break;
        }

        while(pixelPerBar * _barStep < MIN_LINE_SPACING) {
            _barStep *= 2;
        }
    }

    int linesOnDisplay = _subdivisions == 1 ? barsOnDisplay / _barStep : _subdivisions*barsOnDisplay;
    if(_lines.size() < linesOnDisplay) {
        std::size_t old = _lines.size();
        _lines.reserve(linesOnDisplay);

        for(std::size_t l=old; l<_lines.capacity(); ++l) {
            std::unique_ptr<GridLine> gl = std::make_unique<GridLine>();
            //create label
            gl->number = lv_label_create(_numberBackground);
            lv_obj_set_size(gl->number, 80, lv_font_get_line_height(&lv_font_montserrat_28));
            lv_obj_add_style(gl->number, &_numberFont, 0);

            //create line
            gl->line = lv_line_create(_host);
            lv_obj_add_style(gl->line, &Style::gridLine, 0);
            _lines.push_back(std::move(gl));
        }
    }

    for(auto &l : _lines) l->hide();

    double nudgeBars = static_cast<double>(_nudge) / tl.framesPerBar();
    int firstBar = static_cast<int>(Math::floor(nudgeBars));
    // float fractionalBar = static_cast<float>(nudgeBars - static_cast<float>(firstBar));
    int y = _y+30; //+30 - for number background

    if(_subdivisions == 1) {
        int firstIndex = static_cast<int>(Math::ceil(static_cast<double>(firstBar) / _barStep)) * _barStep;
        //draw only bars and numbers
        for(int i=0; i<linesOnDisplay; ++i) {
            GridLine *l = _lines[i].get();

            int barIndex = firstIndex + i * _barStep;
            int xpos = Math::round(pixelPerBar * (barIndex - nudgeBars));
            int barNumber = barIndex + 1;

            lv_obj_set_pos(l->number, xpos, 1);
            lv_label_set_text_fmt(l->number, "%d", barNumber);
            lv_obj_set_style_text_font(l->number, &lv_font_montserrat_28, 0);

            l->points[0] = {_x + xpos, y};
            l->points[1] = {_x + xpos, y + _height - 30};

            lv_line_set_points(l->line, l->points, 2);
            l->show();
        }
    } else {
        int firstIndex = static_cast<int>(Math::floor(nudgeBars * _subdivisions));

        for(int i = 0; i < linesOnDisplay; ++i) {
            GridLine *l = _lines[i].get();

            int index = firstIndex + i;
            int barNumber = static_cast<int>(Math::floor(static_cast<double>(index) / _subdivisions));
            int rem = index - barNumber * _subdivisions;
            bool isBarLine = (rem == 0);
            int xpos = Math::round(pixelPerBar * ((static_cast<double>(index) / _subdivisions) - nudgeBars));

            lv_obj_set_pos(l->number, xpos, 1);

            if(isBarLine) {
                lv_label_set_text_fmt(l->number, "%d", barNumber + 1);
                lv_obj_set_style_text_font(l->number, &lv_font_montserrat_28, 0);
            } else {
                std::string text = std::to_string(rem+1) + "/" + std::to_string(_subdivisions);
                lv_label_set_text_fmt(l->number, "%s", text.c_str());
                lv_obj_set_style_text_font(l->number, &lv_font_montserrat_18, 0);
            }

            l->points[0] = {_x + xpos, y};
            l->points[1] = {_x + xpos, y + _height - 30};

            lv_line_set_points(l->line, l->points, 2);
            l->show();
        }
    }

    if(_loop.visible) {
        updateLoopMarkers(*_zoomPtr);
    }
}

void Timeline::updatePlayhead(slr::frame_t position) {
    slr::TimelineView & tl = slr::TimelineView::getTimelineView();
    // position = tl.elapsed();
    int framesPerBar = tl.framesPerBar();
    int pixPerBar = UIUtility::pixelPerBar(*_zoomPtr);
    float framesPerPixel = (float)pixPerBar / tl.framesPerBar();
    float res = Math::round(framesPerPixel*(position-_nudge));

    _playhead.points[0] = {res, _y+30}; 
    _playhead.points[1] = {res, _height};
    lv_line_set_points(_playhead.line, &_playhead.points[0], 2);
    lv_obj_invalidate(_playhead.line);
}

void Timeline::GridLine::show() {
    lv_obj_clear_flag(number, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(line, LV_OBJ_FLAG_HIDDEN);
}
void Timeline::GridLine::hide() {
    lv_obj_add_flag(number, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(line, LV_OBJ_FLAG_HIDDEN);
}

void Timeline::showLoopMarkers() {
    _loop.show();
}

void Timeline::hideLoopMarkers() {
    _loop.hide();
}

void Timeline::moveToFront() {
    // lv_obj_move_to_index(_loop.filler, -1);
    // lv_obj_move_to_index(_loop.lineStart, -1);
    // lv_obj_move_to_index(_loop.lineEnd, -1);
    // lv_obj_move_to_index(_loop.handleStart->lvhost(), -1);
    // lv_obj_move_to_index(_loop.handleEnd->lvhost(), -1);
    // for(auto &l : _lines) {
    //     lv_obj_move_to_index(l.line, -1);
    // }
}

void Timeline::updateLoopMarkers(float hzoom) {
    //filler, linestart, lineend, handlestart, handleend
    slr::TimelineView &tl = slr::TimelineView::getTimelineView();

    long long int stf = static_cast<long long int>(tl.loopStartFrame()) - _nudge;
    long long int etf = static_cast<long long int>(tl.loopEndFrame()) - _nudge; 

    
    float start = UIUtility::frameToPixel(stf, hzoom);
    float end = UIUtility::frameToPixel(etf, hzoom);

    bool startVisible = stf >= 0 && start < _width;
    bool endVisible = etf >= 0 && end < _width;

    int y = _y+30;

    if(!startVisible && endVisible) {
        lv_obj_add_flag(_loop.lineStart, LV_OBJ_FLAG_HIDDEN);
        _loop.handleStart->hide();
        
        _loop.pointsEnd[0] = {end, y};
        _loop.pointsEnd[1] = {end, y+_height};
        lv_line_set_points(_loop.lineEnd, &_loop.pointsEnd[0], 2);

        lv_obj_set_pos(_loop.handleEnd->lvhost(), end-Layout::TIMELINE_LOOP_HANDLE_W, _height-Layout::TIMELINE_LOOP_HANDLE_H);

        lv_obj_set_size(_loop.filler, end-_x, y+_height-30);
        lv_obj_set_pos(_loop.filler, _x, y);
    } else if(startVisible && endVisible) {
        _loop.show();

        _loop.pointsStart[0] = {_x+start, y};
        _loop.pointsStart[1] = {_x+start, y+_height};
        lv_line_set_points(_loop.lineStart, &_loop.pointsStart[0], 2);
    
        _loop.pointsEnd[0] = {end, y};
        _loop.pointsEnd[1] = {end, y+_height};
        lv_line_set_points(_loop.lineEnd, &_loop.pointsEnd[0], 2);
        
        lv_obj_set_size(_loop.filler, end-(_x+start), y+_height-30);
        lv_obj_set_pos(_loop.filler, _x+start, y);
        lv_obj_set_pos(_loop.handleStart->lvhost(), _x+start, y);
        lv_obj_set_pos(_loop.handleEnd->lvhost(), end-Layout::TIMELINE_LOOP_HANDLE_W, _height-Layout::TIMELINE_LOOP_HANDLE_H);
        
    } else if(startVisible && !endVisible) {
        lv_obj_add_flag(_loop.lineEnd, LV_OBJ_FLAG_HIDDEN);
        _loop.handleEnd->hide();

        _loop.pointsStart[0] = {_x+start, y};
        _loop.pointsStart[1] = {_x+start, y+_height};
        lv_line_set_points(_loop.lineStart, &_loop.pointsStart[0], 2);
    
        lv_obj_set_pos(_loop.handleStart->lvhost(), _x+start, y+30);

        lv_obj_set_size(_loop.filler, _width-(_x+start), y+_height-30);
        lv_obj_set_pos(_loop.filler, _x+start, y);
    }
}

Timeline::LoopHandle::LoopHandle(BaseWidget * parent, Timeline * tl, LoopThings *lparent) :
    BaseWidget(parent, false),
    _lparent(lparent),
    _tl(tl)
{
    _lvhost = lv_obj_create(tl->_host);

    dragCallback(std::bind(&Timeline::LoopHandle::handleDrag, this, std::placeholders::_1));
    touchDownCallback([](const GestLib::TouchDownEvent &td) -> bool {
        return true;
    });
    touchUpCallback([](const GestLib::TouchUpEvent &tu) -> bool {
        return true;
    }) ;
}

Timeline::LoopHandle::~LoopHandle() {
    lv_obj_delete(_lvhost);
}

bool Timeline::LoopHandle::handleDrag(const GestLib::DragGesture &drag) {
    switch(drag.state) {
        case(GestLib::GestureState::Start): {

        } break;
        case(GestLib::GestureState::Move): { 
            int cx = drag.x - lv_obj_get_x(_tl->_host);

            bool isEndHandle = (this == _lparent->handleEnd.get());
            
            lv_obj_set_x(lvhost(), 
                isEndHandle ? 
                cx - Layout::TIMELINE_LOOP_HANDLE_W : 
                cx
            );

            if(isEndHandle) {
                _lparent->pointsEnd[0] = {cx, _tl->_y+30};
                _lparent->pointsEnd[1] = {cx, _tl->_height};
                lv_line_set_points(_lparent->lineEnd, &_lparent->pointsEnd[0], 2);

                //change only width of filler
                int origx = lv_obj_get_x(_lparent->filler);
                lv_obj_set_width(_lparent->filler, cx-origx);
            } else {
                _lparent->pointsStart[0] = {cx, _tl->_y+30};
                _lparent->pointsStart[1] = {cx, _tl->_height};
                lv_line_set_points(_lparent->lineStart, &_lparent->pointsStart[0], 2);
                
                //change
                int origwidth = lv_obj_get_width(_lparent->filler);
                int origx = lv_obj_get_x(_lparent->filler);
                int diff = origx - cx;
                lv_obj_set_x(_lparent->filler, cx);
                lv_obj_set_width(_lparent->filler, origwidth + diff);
            }
        } break;
        case(GestLib::GestureState::End): {
            int cx = drag.x - lv_obj_get_x(_tl->_host);

            slr::frame_t res = UIUtility::pixelToFrame(cx, *(_tl->_zoomPtr));
            LOG_WARN("New loop point: %lu", res);
            //TODO: Snap to grid
            slr::TimelineView & tl = slr::TimelineView::getTimelineView();
            bool isStartHandle = (this != _lparent->handleEnd.get());
            if(isStartHandle) {
                auto act = std::make_unique<slr::Actions::LoopPosition>();
                act->start = res;
                act->end = tl.loopEndFrame();
                slr::EmitAction(std::move(act));
            } else {
                auto act = std::make_unique<slr::Actions::LoopPosition>();
                act->start = tl.loopStartFrame();
                act->end = res;
                slr::EmitAction(std::move(act));
            }
        } break;
    }
    return true;
}

Timeline::LoopThings::~LoopThings() {
    lv_obj_delete(filler);
    lv_obj_delete(lineStart);
    lv_obj_delete(lineEnd);
}

void Timeline::LoopThings::show() {
    lv_obj_clear_flag(filler, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(lineStart, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(lineEnd, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(handleStart->lvhost(), LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(handleEnd->lvhost(), LV_OBJ_FLAG_HIDDEN);
    visible = true;
}

void Timeline::LoopThings::hide() {
    lv_obj_add_flag(filler, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(lineStart, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(lineEnd, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(handleStart->lvhost(), LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(handleEnd->lvhost(), LV_OBJ_FLAG_HIDDEN);
    visible = false;
}

}