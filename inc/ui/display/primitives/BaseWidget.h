// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lvgl.h"
#include "gestlib/GestLib.h"

#include <vector>
#include <algorithm>

namespace UI {

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


    void hide();
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
    

    protected:
    BaseWidget * _parent;
    std::vector<BaseWidget*> _childs;

    bool _isRoot;
    bool _hasHost;
    lv_obj_t * _lvhost;

    GestureFlags _flags;

    virtual bool handleTouchDown(GestLib::TouchDownEvent & touchDown);
    virtual bool handleTouchUp(GestLib::TouchUpEvent & touchUp);
    virtual bool handleTap(GestLib::TapGesture & tap);
    virtual bool handleHold(GestLib::HoldGesture & hold);
    virtual bool handleDoubleTap(GestLib::DoubleTapGesture & dtap);
    virtual bool handleDrag(GestLib::DragGesture & drag);
    virtual bool handleSwipe(GestLib::SwipeGesture & swipe);
    virtual bool handleDTSwipe(GestLib::DTSwipeGesture & swipe);
    virtual bool handleDTCircular(GestLib::DTCircularGesture & swipe);

    uint64_t _lastPolledUIVersion;
    private:
    // BaseWidget * _parent;
    // std::vector<BaseWidget*> _childs;

    // bool _isRoot;
    // bool _hasHost;
    // lv_obj_t * _lvhost;

};

} //namespace UI