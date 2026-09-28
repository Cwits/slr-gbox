// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display2/elements/RootWindow.h"

#include "display2/primitives/Button.h"
#include "display2/primitives/Label.h"

#include "display2/elements/TopPanel.h"
#include "display2/elements/Grid.h"
#include "display2/elements/BottomPanel.h"

#include "display2/utility/DefaultStyle.h"
#include "display2/utility/DefaultColors.h"

#include "common/logger.h"

#include <iostream>
#include <algorithm>
#include <vector>
#include <memory>

namespace Display {

RootWindow::RootWindow(lv_obj_t * screen) : 
    BaseWidget(screen),
    _ctx(this)
{
    _lvhost = screen;
    
    lv_obj_set_style_bg_color(_lvhost, DefaultColors::Background, LV_PART_MAIN);
    lv_obj_set_style_radius(_lvhost, 0, 0);
    lv_obj_set_style_margin_all(_lvhost, 0, 0);
    lv_obj_set_style_pad_all(_lvhost, 0, 0);
    lv_obj_set_style_border_width(_lvhost, 0, 0);

    _topPanel = std::make_unique<TopPanel>(this, &_ctx);
    _grid = std::make_unique<Grid>(this, &_ctx);
    _bottomPanel = std::make_unique<BottomPanel>(this, &_ctx);
}

RootWindow::~RootWindow() {
}


bool RootWindow::handleGesture(GestLib::Gesture & gesture) {
    BaseWidget * globtarget = nullptr;
    if(gesture.type == GestLib::Gestures::TouchDown) {
        int x = gesture.touchDown.x;
        int y = gesture.touchDown.y;

        BaseWidget * node = this;
        
        // Popup * pop = nullptr;
        // bool popActive = false;
        // for(Popup * p : _popups) {
        //     if(p->active()) {
        //         pop = p;
        //         popActive = true;
        //         break;
        //     }
        // }

        // if(popActive) {
        //     node = pop;
        // } else {

        //     if(y <= Layout::TOP_PANEL_HEIGHT) {
        //         //look in top panel
        //         node = _topPanel.get();
        //     } else if(y > Layout::TOP_PANEL_HEIGHT && y < (Layout::WORKSPACE_HEIGHT+Layout::TOP_PANEL_HEIGHT)) {
        //         //look in workspace
        //         node = getSwitchViewTarget(_currentView);
        //     } else {
        //         //look in bottom panel
        //         //node = _bottomPanel;
        //         node = _bottomPanel.get();
        //     }

        // }

        BaseWidget * target = hitTest(node, x, y);
        
        // if(target == nullptr && popActive) {
        //     pop->hide();
        //     pop->deactivate();
        //     return false;
        // }
        
        _gestureTarget = target;
        _initialGestureTarget = target;

        if(_initialGestureTarget)
            globtarget = _initialGestureTarget;
            // _initialGestureTarget->handleGesture(gesture);
        // return true;
    } else if(gesture.type == GestLib::Gestures::TouchUp) {
        if(_initialGestureTarget)
            globtarget = _initialGestureTarget;
            // _initialGestureTarget->handleGesture(gesture);
        
        _gestureTarget = nullptr;
        _initialGestureTarget = nullptr;
        // return true;
    } else {
        //handle gesture by target if there is one
        if(!_gestureTarget) return false;

        //check that can handle gesture, of not - that go up in hierarchy till main(fallback)
        globtarget = _gestureTarget;
    }

    if(!globtarget) {
        LOG_WARN("Hittest target is nullptr in %s gesture", GestLib::gestureToText(gesture.type).c_str());
        return false;
    }

    if(!globtarget->canHandleGesture(gesture.type)) {
        if(!globtarget->parent()) return false;

        BaseWidget * node = globtarget->parent();
        while(node) {
            if(node->canHandleGesture(gesture.type)) {
                globtarget = node;
                break;
            }
            node = node->parent();
            if(node == nullptr) 
                return false;
        }
    }

    bool ret = globtarget->handleGesture(gesture);
    return ret;
}

//transfers ongoing gesture to different view(e.g. from browser to grid)
void RootWindow::transferGesture(BaseWidget * target, GestLib::Gestures gesture) {
    if(!target->canHandleGesture(gesture)) {
        LOG_WARN("Target can't handle gesture %s", GestLib::gestureToText(gesture).c_str());
        return;
    }

    // if(target == _dragViewSelector.get()) {
    //     _gestureTarget = _dragViewSelector.get();
    // } else if(target == _gridView.get()) {
    //     _gestureTarget = _gridView.get();
    // } else if(target == _browser.get()) {
    //     _gestureTarget = _browser.get();
    // } else if(target == _unitView.get()) {
    //     _gestureTarget = _unitView.get();
    // } else if(target == _modEngineView.get()) {
    //     _gestureTarget = _modEngineView.get();
    // } else {
    //     LOG_ERROR("Target not handled");
    // }
}

bool RootWindow::cancleGesture(BaseWidget * widget) {
    if(_gestureTarget == widget) {
        _gestureTarget = nullptr;
        _initialGestureTarget = nullptr;
        return true;
    }

    LOG_WARN("Failed to cancle gesture");
    return false;
}

BaseWidget * RootWindow::hitTest(BaseWidget * node, int x, int y) {
    static auto zsort = [](const BaseWidget *op1, const BaseWidget *op2) -> bool {
        return (op1->getZ() < op2->getZ());
    };

    std::vector<BaseWidget*> sorted;
    sorted = node->children();
    std::sort(sorted.begin(), sorted.end(), zsort);

    // LOG_INFO("hit test x: %d, y: %d", x, y);

    //room for improve - collect all targets within acceptable region and decide which to pick based on Z
    for(auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
        auto* child = *it;
        if(!child->visible()) continue; //sometimes visible() failed because of nullptr lvhost??

        int notAbsX = lv_obj_get_x(node->lvhost());
        int notAbsY = lv_obj_get_y(node->lvhost());
        if(!child->contains(x-notAbsX, y-notAbsY))
            continue;

        if(auto* target = hitTest(child, x-notAbsX, y-notAbsY)) {
            return target; // A deeper widget handled it
        }

        // if (child->canHandleGesture(gesture)) {
            return child;
        // }
    }

    // If no child handled it, test current node.
    //need to do the same notAbsY magic??
    if(node->contains(x, y)) {
        return node;
    }

    return nullptr; // No match
}


} //namespace Display
