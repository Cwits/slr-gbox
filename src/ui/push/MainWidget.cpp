// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/push/MainWidget.h"

#include "ui/push/PadLayoutSelector.h"
#include "ui/push/GridWidget.h"
#include "ui/push/ModuleWidget.h"
#include "ui/push/BrowserWidget.h"

#include "push/PushContext.h"
#include "push/PushPainter.h"
#include "push/helper.h"

#include "logger.h"

namespace PushUI {

const PushLib::ButtonCallbackMap<MainWidget> MainWidget::_buttonsCallback = {
    {PushLib::Button::User, &MainWidget::userBtnClb},
    {PushLib::Button::Device, &MainWidget::deviceBtnClb},
    {PushLib::Button::Browser, &MainWidget::browserBtnClb},
    {PushLib::Button::Play, &MainWidget::playButtonClb},
    {PushLib::Button::Record, &MainWidget::recordButtonClb},
    {PushLib::Button::Scale, &MainWidget::scaleSelector},
    {PushLib::Button::Layout, &MainWidget::layoutSelector},
};

static int playColor = 1;

MainWidget::MainWidget(PushLib::PushContext * const pctx) :
    PushLib::Widget(nullptr)
    // _currentView(PushView::Grid)
{
    _puictx._pctx = pctx;
    _puictx._mainWidget = this;

    _padLayoutSelector = std::make_unique<PadLayoutSelector>(this, &_puictx);
    _gridWidget = std::make_unique<GridWidget>(this, &_puictx);
    _moduleWidget = std::make_unique<ModuleWidget>(this, &_puictx);
    _browserWidget = std::make_unique<BrowserWidget>(this, &_puictx);

    // colorButtons();
    
    PushLib::ButtonColor play;
    play.btn = PushLib::Button::Play;
    play.color = playColor;
    play.anim = PushLib::LedAnimation();
    pctx->setButtonColor(play);

    switchToView(PushView::Grid);
}

MainWidget::~MainWidget() {

}

PushLib::BoundingBox MainWidget::invalidate() { //return BoundingBox of area that has to be redrawn
    PushLib::BoundingBox ret;
    if(_viewSwitched) {
        ret.x = 0;
        ret.y = 0;
        ret.h = PushLib::DISPLAY_HEIGHT;
        ret.w = PushLib::DISPLAY_WIDTH;
        _viewSwitched = false;
    } else {
        //invalidate?
        ret = widgetFromView(_currentView)->invalidate();
    }
    return ret;
}

void MainWidget::paint(PushLib::Painter &painter) {
    /* 
        best case scenario - invalidate which objects are dirty and redraw only that specific area, but not whole thing...
        plus... somehow need to tell push core where bounding box changed in order for him not to recode whole screen?
        step 1:
            make invalidating of elements in drawable space
        
        step 2:
            make invalidating before preparing sendable batch in PushDisplay::updateFrame
    */

    widgetFromView(_currentView)->paint(painter);
}

bool MainWidget::handleButton(PushLib::ButtonEvent &ev) {
    return widgetFromView(_currentView)->handleButton(ev);
}

bool MainWidget::handleEncoder(PushLib::EncoderEvent &ev) {
    // std::string text;
    // if(ev.type == PushLib::EncoderEventType::Touched) text = "Touched";
    // else if(ev.type == PushLib::EncoderEventType::Moved) text = "Moved";
    // else if(ev.type == PushLib::EncoderEventType::Released) text = "Released";
    // LOG_WARN("Encoder Event: %s %d %d", text.c_str(), static_cast<int>(ev.encoder), ev.delta);
    return widgetFromView(_currentView)->handleEncoder(ev);
}

PushLib::Widget * MainWidget::widgetFromView(PushView view) {
    Widget * ret = nullptr;
    switch(view) {
        case(PushView::Grid): ret = _gridWidget.get(); break;
        case(PushView::Module): ret = _moduleWidget.get(); break;
        case(PushView::Editor): break;
        case(PushView::Patch): break;
        case(PushView::StepSequencer): break;
        case(PushView::ModMatrix): break;
        case(PushView::Browser): ret = _browserWidget.get(); break;
        case(PushView::Metronome): break;
        case(PushView::ScaleSelector): ret = _padLayoutSelector.get(); break;
        case(PushView::PushInternalSettings): break;
        case(PushView::SLRSettings): break;
    }

    return ret;
}

void MainWidget::switchToView(PushView view) {
    if(view == _currentView) {
        switchToView(_previousView);
        return;
    }

    _previousView = _currentView;
    _currentView = view;
    _viewSwitched = true;
    colorButtons();
}

void MainWidget::goToPreviousView() {
    switchToView(_previousView);
}

const PushView MainWidget::previousView() const {
    return _previousView;
}

const PushView MainWidget::currentView() const {
    return _currentView;
}

bool MainWidget::handleDefaultButton(PushLib::ButtonEvent &ev) {
    //use switch later...
    const auto res = _buttonsCallback.find(ev.button);
    if(res != _buttonsCallback.end()) {
        const auto cb = res->second;
        return (this->*cb)(ev);
    }

    return false;
}

bool MainWidget::handleDefaultEncoder(PushLib::EncoderEvent &ev) {
    return false;
}

std::vector<PushLib::ButtonColor> MainWidget::buttonsColors() {
    return PushHelper::buttonColorsFromMap<MainWidget>(MainWidget::_buttonsCallback);
}

void MainWidget::colorButtons() {
    // _puictx.pctx()->clearButtonColors();
    // std::vector<PushLib::ButtonColor> map = PushHelper::buttonColorsFromMap<MainWidget>(MainWidget::_buttonsCallback);
    // _puictx.pctx()->setButtonsColors(map);
    //1. get from current view
    //2. get main
    //3. make diff where
    //  base is from view
    //  rest is from main

    std::vector<PushLib::ButtonColor> viewMap = widgetFromView(currentView())->buttonsColors();
    std::vector<PushLib::ButtonColor> defaultMap = buttonsColors();

    std::vector<PushLib::ButtonColor> result;
    result.reserve(viewMap.size() + defaultMap.size());

    //diff
    for(auto &v : viewMap) {
        result.push_back(v);
    }

    for(auto &v : defaultMap) {
        bool exists = false;
        for(auto &e : result) {
            if(v.btn == e.btn) {
                exists = true;
                break;
            }
        }
        if(exists) continue;

        result.push_back(v);
    }

    _puictx.pctx()->clearButtonColors();
    _puictx.pctx()->setButtonsColors(result);
}

/* --------------------    HANDLES    ------------------- */

bool MainWidget::userBtnClb(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;
    
    switchToView(PushView::Grid);
    return true;
}

bool MainWidget::deviceBtnClb(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;
    
    switchToView(PushView::Module);
    return true;
}

bool MainWidget::browserBtnClb(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;
    
    switchToView(PushView::Browser);
    return true;
}

bool MainWidget::scaleSelector(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;
    
    switchToView(PushView::ScaleSelector);
    return true;
}

bool MainWidget::layoutSelector(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;
    
    switchToView(PushView::ScaleSelector);
    return true;
}

bool MainWidget::playButtonClb(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;
    
    LOG_INFO("test Play btn");
    playColor++;
    PushLib::ButtonColor play;
    play.btn = PushLib::Button::Play;
    play.color = playColor;
    play.anim = PushLib::LedAnimation();
    _puictx.pctx()->setButtonColor(play);

    return true;
}

bool MainWidget::recordButtonClb(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;
    
    LOG_INFO("test success");
    playColor++;
    PushLib::ButtonColor play;
    play.btn = PushLib::Button::Play;
    play.color = playColor;
    play.anim = PushLib::LedAnimation();
    _puictx.pctx()->setButtonColor(play);

    return true;
}

} //namespace PushUI