// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "gestlib/GestLib.h"
#include "lvgl.h"

#include <vector>
#include <algorithm>
#include <functional>

namespace Display {

struct BaseWidget {
    struct GestureFlags {
        bool isTouchDown;
        bool isTouchUp;
        bool isTap;
        bool isHold;
        bool isDrag;
        bool isSwipe;
        bool isDoubleTap;
        bool isDoubleTapSwipe;
        bool isDoubleTapCircular;
        bool isZoom; //-
        bool isTwoFingerTap; //-
        bool isTwoFingerSwipe; //-
        bool isThreeFingerTap; //-
        bool isThreeFingerSwipe; //-
    };
    
    BaseWidget(BaseWidget * parent, bool hasHost = false, bool addAsChild = true);
    explicit BaseWidget(lv_obj_t * parent);
    virtual ~BaseWidget();

    bool canHandleGesture(GestLib::Gestures gesture);
    bool handleGesture(GestLib::Gesture & gesture);


    virtual void setSize(lv_coord_t w, lv_coord_t h);
    virtual void setPos(lv_coord_t x, lv_coord_t y);
    void setColor(lv_color_t color);
    int getX();
    int getY();
    int width();
    int height();
    int getZ() const;


    virtual void hide();
    virtual void show();
    bool visible();

    lv_obj_t * lvhost() const { return _lvhost; }

    void addChild(BaseWidget * child) { _childs.push_back(child); }
    std::vector<BaseWidget*> & children() { return _childs; }
    void hideAllChilds();    

    bool contains(int x, int y);
    bool containsX(int x);
    bool containsY(int y);

    BaseWidget * parent() const { return _parent; }

    virtual void pollUIUpdate() {}

    inline void pollChildsUIUpdate() {
        std::for_each(_childs.begin(), _childs.end(), [](BaseWidget *w) {
            w->pollUIUpdate();
        });
    }

    //if _lastPolledUIVersion != other -> save new and return false
    bool isSameUIVersion(uint64_t other) {
        if(other == _lastPolledUIVersion) return true;
        
        _lastPolledUIVersion = other;
        return false;
    }
    
    
    void touchDownCallback(std::function<void(const GestLib::TouchDownEvent &)> onTouchDown);
    void touchUpCallback(std::function<void(const GestLib::TouchUpEvent &)> onTouchUp);
    void tapCallback(std::function<void(const GestLib::TapGesture &)> onTap);
    void holdCallback(std::function<void(const GestLib::HoldGesture &)> onHold);
    void dragCallback(std::function<void(const GestLib::DragGesture &)> onDrag);
    void swipeCallback(std::function<void(const GestLib::SwipeGesture &)> onSwipe);
    void doubleTapCallback(std::function<void(const GestLib::DoubleTapGesture &)> onDT);
    

    protected:
    BaseWidget * _parent;
    std::vector<BaseWidget*> _childs;

    bool _isRoot;
    bool _hasHost;
    lv_obj_t * _lvhost;

    GestureFlags _flags;
    
    std::function<void(const GestLib::TouchDownEvent &)> _onTouchDown;
    std::function<void(const GestLib::TouchUpEvent &)> _onTouchUp;
    std::function<void(const GestLib::TapGesture &)> _onTap;
    std::function<void(const GestLib::HoldGesture &)> _onHold;
    std::function<void(const GestLib::DragGesture &)> _onDrag;
    std::function<void(const GestLib::SwipeGesture &)> _onSwipe;
    std::function<void(const GestLib::DoubleTapGesture &)> _onDoubleTap;

    virtual bool handleTouchDown(const GestLib::TouchDownEvent & touchDown);
    virtual bool handleTouchUp(const GestLib::TouchUpEvent & touchUp);
    virtual bool handleTap(const GestLib::TapGesture & tap);
    virtual bool handleHold(const GestLib::HoldGesture & hold);
    virtual bool handleDoubleTap(const GestLib::DoubleTapGesture & dtap);
    virtual bool handleDrag(const GestLib::DragGesture & drag);
    virtual bool handleSwipe(const GestLib::SwipeGesture & swipe);
    virtual bool handleDTSwipe(const GestLib::DTSwipeGesture & swipe);
    virtual bool handleDTCircular(const GestLib::DTCircularGesture & swipe);

    uint64_t _lastPolledUIVersion;
    private:
    // BaseWidget * _parent;
    // std::vector<BaseWidget*> _childs;

    // bool _isRoot;
    // bool _hasHost;
    // lv_obj_t * _lvhost;

};


} //namespace UI