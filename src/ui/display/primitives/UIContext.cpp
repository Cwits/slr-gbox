// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/primitives/UIContext.h"

#include "ui/display/MainWindow.h"
#include "ui/display/TopPanel.h"
#include "ui/display/BottomPanel.h"
#include "ui/display/GridView.h"
#include "ui/display/ModuleView.h"
#include "ui/display/Browser.h"

namespace UI {

void UIContext::switchToView(MainView view) {
    _mainWindow->switchToView(view);
}

MainView UIContext::previousView() {
    return _mainWindow->previousView();
}

void UIContext::transferGesture(MainView view, GestLib::Gestures gesture) {
    _mainWindow->transferGesture(view, gesture);
}

void UIContext::clearHitTestTarget() {
    _mainWindow->clearHittestTarget();
}

bool UIContext::cancleGesture(BaseWidget * widget) {
    return _mainWindow->cancleGesture(widget);
}

void UIContext::floatingText(bool warn, std::string text) {
    if(warn) _mainWindow->floatingTextWarning(text);
    else _mainWindow->floatingTextRegular(text);
}

void UIContext::setLastSelected(UnitUIBase * mod) {
    if(mod == nullptr) {
        _lastSelectedModule = nullptr;
        
        lv_obj_add_flag(_gridView->_control->_lastSelectedRect, LV_OBJ_FLAG_HIDDEN);
    } else {
        _lastSelectedModule = mod;
    }
}

UnitUIBase * UIContext::getLastSelected() {
    return _lastSelectedModule;
}

float UIContext::gridHorizontalZoom() {
    return _gridView->hZoom();
}

BaseWidget * UIContext::topPanel() { return _topPanel; }
BaseWidget * UIContext::bottomPanel() { return _bottomPanel; }
BaseWidget * UIContext::gridControl() { return _gridView->_control; }
BaseWidget * UIContext::grid() { return _gridView->_grid; }
BaseWidget * UIContext::moduleView() { return _module; }
BaseWidget * UIContext::browser() { return _browser; }

}