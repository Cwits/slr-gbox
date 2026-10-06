// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display/primitives/BaseWidget.h"
#include "display/utility/DefaultStyles.h"

#include "common/logger.h"
#include <algorithm>

namespace UI {

/*
    if hasHost = true than BaseWidget will call lv_obj_create(parent->lvhost())
 */
BaseWidget::BaseWidget(BaseWidget * parent, bool hasHost, bool addAsChild) : 
    _parent(parent), 
    _isRoot(false),
    _hasHost(hasHost),
    _lvhost(nullptr)
{
    if(_hasHost) {
        if(parent == nullptr) _lvhost = lv_obj_create(nullptr);
        else _lvhost = lv_obj_create(parent->lvhost());
        hide();
    }

    if(_parent && addAsChild) {
        _parent->addChild(this);
    }
    
    _flags = { false, false,
                false, false, false, 
                false, false, false,
                false, false, false,
                false, false, false};

    _lastPolledUIVersion = 0;
}

BaseWidget::BaseWidget(lv_obj_t * parent) :
    _parent(nullptr),
    _isRoot(true),
    _hasHost(true),
    _lvhost(nullptr)
{
    _lvhost = parent;
}

BaseWidget::~BaseWidget() {
    if(_parent != nullptr) {
        std::vector<BaseWidget*> & childs = _parent->children();
        if(childs.size() > 0) {
            childs.erase(std::remove_if(childs.begin(), childs.end(), [this](const BaseWidget * ptr){
                if(this == ptr) return true;
                else return false;
            }), childs.end());
        }
    }

    if(_hasHost && !_isRoot) {
        lv_obj_delete(_lvhost);
    }
}

void BaseWidget::setSize(lv_coord_t w, lv_coord_t h) {
    if(!_lvhost && !_isRoot) return;
    lv_obj_set_size(_lvhost, w, h);
}

void BaseWidget::setPos(lv_coord_t x, lv_coord_t y) {
    if(!_lvhost && !_isRoot) return;
    lv_obj_set_pos(_lvhost, x, y);
}

void BaseWidget::setColor(lv_color_t color) {
    if(!_lvhost) return;
    lv_obj_set_style_bg_color(_lvhost, color, LV_PART_MAIN);
}

void BaseWidget::addStyle(const lv_style_t *style) {
    if(!_lvhost) return;
    lv_obj_add_style(_lvhost, style, 0);
}

void BaseWidget::noScroll() {
    if(!_lvhost) return;
    lv_obj_set_scrollbar_mode(_lvhost, LV_SCROLLBAR_MODE_OFF);
}

int BaseWidget::getX() {
    if(!_lvhost) return 0;
    return lv_obj_get_x(_lvhost);
}

int BaseWidget::getY() {
    if(!_lvhost) return 0;
    return lv_obj_get_y(_lvhost);
}

int BaseWidget::getZ() const {
    if(!_lvhost) return 0;
    return lv_obj_get_index(_lvhost);
}

int BaseWidget::width() {
    if(!_lvhost) return 0;
    return lv_obj_get_width(_lvhost);
}

int BaseWidget::height() {
    if(!_lvhost) return 0;
    return lv_obj_get_height(_lvhost);
}

void BaseWidget::hide() {
    if(!_lvhost && !_isRoot) return;
    lv_obj_add_flag(_lvhost, LV_OBJ_FLAG_HIDDEN);
}

void BaseWidget::show() {
    if(!_lvhost && !_isRoot) return;
    lv_obj_clear_flag(_lvhost, LV_OBJ_FLAG_HIDDEN);
}

bool BaseWidget::visible() {
    if(!_lvhost) {
        LOG_WARN("_lvhost is nullptr");
        return false;
    }
    return lv_obj_has_flag(_lvhost, LV_OBJ_FLAG_HIDDEN) ? false : true;
}

bool BaseWidget::contains(int x, int y) {
    if(!_lvhost) {
        LOG_WARN("_lvhost is nullptr");
        return false;
    }
    int xpos = lv_obj_get_x(_lvhost);
    int ypos = lv_obj_get_y(_lvhost);
    int width = lv_obj_get_width(_lvhost);
    int height = lv_obj_get_height(_lvhost);

    bool t1 = x >= xpos;
    bool t2 = x <= (xpos+width);
    bool t3 = y >= ypos;
    bool t4 = y <= (ypos+height);

    if(t1 && t2 && t3 && t4) return true;
    else return false;
}

bool BaseWidget::containsX(int x) {
    if(!_lvhost) {
        LOG_WARN("_lvhost is nullptr");
        return false;
    }
    int xpos = lv_obj_get_x(_lvhost);
    int width = lv_obj_get_width(_lvhost);
    bool t1 = x >= xpos;
    bool t2 = x <= (xpos+width);

    if(t1 && t2) return true;
    else return false;
}

bool BaseWidget::containsY(int y) {
    if(!_lvhost) {
        LOG_WARN("_lvhost is nullptr");
        return false;
    }
    int ypos = lv_obj_get_y(_lvhost);
    int height = lv_obj_get_height(_lvhost);
    bool t3 = y >= ypos;
    bool t4 = y <= (ypos+height);

    if(t3 && t4) return true;
    else return false;
}

void BaseWidget::hideAllChilds() {
    for(BaseWidget * child : _childs) {
        child->hide();
    }
}

bool BaseWidget::canHandleGesture(GestLib::Gestures gesture) {
    bool ret = false;
    if(gesture == GestLib::Gestures::TouchDown && _flags.isTouchDown) ret = true;
    if(gesture == GestLib::Gestures::TouchUp && _flags.isTouchUp) ret = true;
    if(gesture == GestLib::Gestures::Tap && _flags.isTap) ret = true;
    if(gesture == GestLib::Gestures::Hold && _flags.isHold) ret = true;
    if(gesture == GestLib::Gestures::Drag && _flags.isDrag) ret = true;
    if(gesture == GestLib::Gestures::Swipe && _flags.isSwipe) ret = true;
    if(gesture == GestLib::Gestures::DoubleTap && _flags.isDoubleTap) ret = true;
    if(gesture == GestLib::Gestures::DoubleTapSwipe && _flags.isDoubleTapSwipe) ret = true;
    if(gesture == GestLib::Gestures::DoubleTapCircular && _flags.isDoubleTapCircular) ret = true;
    if(gesture == GestLib::Gestures::Zoom && _flags.isZoom) ret = true;
    if(gesture == GestLib::Gestures::TwoFingerTap && _flags.isTwoFingerTap) ret = true;
    if(gesture == GestLib::Gestures::TwoFingerSwipe && _flags.isTwoFingerSwipe) ret = true;
    if(gesture == GestLib::Gestures::ThreeFingerTap && _flags.isThreeFingerTap) ret = true;
    if(gesture == GestLib::Gestures::ThreeFingerSwipe && _flags.isThreeFingerSwipe) ret = true;
    if(gesture == GestLib::Gestures::Zoom && _flags.isZoom) ret = true;
    return ret;
}

bool BaseWidget::handleGesture(GestLib::Gesture & gesture) {
    bool ret = false;
    switch(gesture.type) {
        case(GestLib::Gestures::TouchDown): ret = handleTouchDown(gesture.touchDown); break;
        case(GestLib::Gestures::TouchUp): ret = handleTouchUp(gesture.touchUp); break;
        case(GestLib::Gestures::Tap): ret = handleTap(gesture.tap); break;
        case(GestLib::Gestures::Hold): ret = handleHold(gesture.hold); break;
        case(GestLib::Gestures::DoubleTap): ret = handleDoubleTap(gesture.doubleTap); break;
        case(GestLib::Gestures::Drag): ret = handleDrag(gesture.drag); break;
        case(GestLib::Gestures::Swipe): ret = handleSwipe(gesture.swipe); break;
        case(GestLib::Gestures::DoubleTapSwipe): ret = handleDTSwipe(gesture.dtSwipe); break;
        case(GestLib::Gestures::DoubleTapCircular): ret = handleDTCircular(gesture.dtCircular); break;
        case(GestLib::Gestures::Zoom): ret = handleZoom(gesture.zoom); break;
    }
    return ret;
}

void BaseWidget::tapCallback(std::function<bool(const GestLib::TapGesture &)> onTap) {
    _onTap = std::move(onTap);
    _flags.isTap = true;
}

void BaseWidget::touchDownCallback(std::function<bool(const GestLib::TouchDownEvent &)> onTouchDown) {
    _onTouchDown = std::move(onTouchDown);
    _flags.isTouchDown = true;
}

void BaseWidget::touchUpCallback(std::function<bool(const GestLib::TouchUpEvent &)> onTouchUp) {
    _onTouchUp = std::move(onTouchUp);
    _flags.isTouchUp = true;
}

void BaseWidget::holdCallback(std::function<bool(const GestLib::HoldGesture &)> onHold) {
    _onHold = std::move(onHold);
    _flags.isHold = true;
}

void BaseWidget::dragCallback(std::function<bool(const GestLib::DragGesture &)> onDrag) {
    _onDrag = std::move(onDrag);
    _flags.isDrag = true;
}

void BaseWidget::swipeCallback(std::function<bool(const GestLib::SwipeGesture &)> onSwipe) {
    _onSwipe = std::move(onSwipe);
    _flags.isSwipe = true;
}
void BaseWidget::doubleTapCallback(std::function<bool(const GestLib::DoubleTapGesture &)> onDT) {
    _onDoubleTap = std::move(onDT);
    _flags.isDoubleTap = true;
}
void BaseWidget::zoomCallback(std::function<bool(const GestLib::ZoomGesture &)> onZoom) {
    _onZoom = std::move(onZoom);
    _flags.isZoom = true;
}
    
bool BaseWidget::handleTap(const GestLib::TapGesture & tap) {
    if(_onTap) { return _onTap(tap); }
    return false;
}

bool BaseWidget::handleTouchDown(const GestLib::TouchDownEvent &down) {
    if(_onTouchDown) { return _onTouchDown(down); }
    return false;
}
bool BaseWidget::handleTouchUp(const GestLib::TouchUpEvent &up) {
    if(_onTouchUp) { return _onTouchUp(up); }
    return false;
}
bool BaseWidget::handleHold(const GestLib::HoldGesture & hold) {
    if(_onHold) { return _onHold(hold); }
    return false;
}
bool BaseWidget::handleDoubleTap(const GestLib::DoubleTapGesture & dtap) {
    if(_onDoubleTap) { return _onDoubleTap(dtap); }
    return false;
}
bool BaseWidget::handleDrag(const GestLib::DragGesture & drag) {
    if(_onDrag) { return _onDrag(drag); }
    return false;
}
bool BaseWidget::handleSwipe(const GestLib::SwipeGesture & swipe) {
    if(_onSwipe) { return _onSwipe(swipe); }
    return false;
}
bool BaseWidget::handleDTSwipe(const GestLib::DTSwipeGesture & swipe) {
    return false;
}
bool BaseWidget::handleDTCircular(const GestLib::DTCircularGesture & swipe) {
    return false;
}
bool BaseWidget::handleZoom(const GestLib::ZoomGesture &zoom) {
    if(_onZoom) { return _onZoom(zoom); }
    return false;
}

}