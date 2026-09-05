// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display/utility/UIContext.h"


#include "display/primitives/Popup.h"
#include "display/popups/DragViewSelector.h"
#include "display/elements/MainWindow.h"
#include "display/elements/TopPanel.h"
#include "display/elements/BottomPanel.h"
#include "display/elements/GridView.h"
#include "display/elements/UnitView.h"
#include "display/elements/Browser.h"
#include "display/elements/StepSequencerView.h"

namespace UI {

void UIContext::switchToView(MainView view) {
    _mainWindow->switchToView(view);
}

MainView UIContext::previousView() {
    return _mainWindow->previousView();
}

void UIContext::transferGesture(BaseWidget * target, GestLib::Gestures gesture) {
    _mainWindow->transferGesture(target, gesture);
}

void UIContext::clearHitTestTarget() {
    _mainWindow->clearHittestTarget();
}

bool UIContext::cancleGesture(BaseWidget * widget) {
    return _mainWindow->cancleGesture(widget);
}

void UIContext::floatingText(bool warn, std::string text) {
    // if(warn) _mainWindow->floatingTextWarning(text);
    // else _mainWindow->floatingTextRegular(text);
    _mainWindow->floatingText(warn, text);
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


void UIContext::registerFrequentUpdate(std::function<void()> clb) {
    _mainWindow->registerFrequentUpdate(std::move(clb));
}

BaseWidget * UIContext::topPanel() { return _topPanel; }
BaseWidget * UIContext::bottomPanel() { return _bottomPanel; }
BaseWidget * UIContext::grid() { return _gridView; }
BaseWidget * UIContext::gridControl() { return _gridView->_control.get(); }
BaseWidget * UIContext::gridGrid() { return _gridView->_grid.get(); }
BaseWidget * UIContext::unitView() { return _unitView; }
BaseWidget * UIContext::browser() { return _browser; }
BaseWidget * UIContext::stepSequencer() { return _stepSequencerView; }
BaseWidget * UIContext::dragSelector() { return _dragSelector; }


}