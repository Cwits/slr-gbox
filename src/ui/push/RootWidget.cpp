// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/push/RootWidget.h"

#include "ui/push/PadLayoutWidget.h"
#include "ui/push/GridWidget.h"
#include "ui/push/ModuleWidget.h"
#include "ui/push/BrowserWidget.h"

#include "push/PushContext.h"
#include "push/PushPainter.h"
#include "push/helper.h"

#include "snapshots/AudioUnitView.h"
#include "core/ModuleManager.h"
#include "core/Events.h"

#include "logger.h"

#include <string>

namespace PushUI {

#define CLBS(x) \
    X(User, userBtnClb) \
    X(Device, deviceBtnClb) \
    X(Browser, browserBtnClb) \
    X(Play, playButtonClb) \
    X(Record, recordButtonClb) \
    X(Scale, scaleSelector) \
    X(Layout, layoutSelector)

const PushLib::ButtonCallbackMap<RootWidget> RootWidget::_buttonsCallback = {
#define X(btn, clb) {PushLib::Button::btn, &RootWidget::clb},
    CLBS(X)
#undef X
};

const std::vector<PushLib::ButtonColor> _colors = {
#define X(btn, clb) { PushLib::Button::btn, PushLib::LedAnimation(), 127 },
    CLBS(X)
#undef X
};

static int playColor = 1;

static RootWidget * _rootInstance = nullptr;

std::vector<PushLib::BoundingBox> _dirtyRegions;

void collectDirty(PushLib::Widget *w, std::vector<PushLib::BoundingBox> &list) {
    if(!w->visible()) return; 

    if(w->dirty()) {
        PushLib::BoundingBox b = w->lastBounds();
        if(!b.empty())
            list.push_back(b);
        
        b = w->bounds();
        if(!b.empty())
            list.push_back(b);
    }
    for(PushLib::Widget *c : w->childs())
        collectDirty(c, list);
};

void traverse(PushLib::Widget *w, PushLib::Painter &p, const PushLib::BoundingBox &clip) {
    if(!w->visible()) return;

    if(!w->bounds().intersects(clip)) return;

    PushLib::BoundingBox newClip = w->bounds().intersect(clip);
    if(newClip.empty()) return;

    p.pushClip(newClip);

    w->sortChildsByZ();

    w->paint(p);

    for(PushLib::Widget *c : w->childs()) 
        traverse(c, p, newClip);

    p.popClip();
};

void finalize(PushLib::Widget *w) {
    w->clearDirty();
    w->updateBounds();

    for(PushLib::Widget *c : w->childs())
        finalize(c);
}

RootWidget::RootWidget(PushLib::PushContext * const pctx) :
    PushLib::Widget(nullptr),
    _currentView(PushView::ERROR)
{
    _rootInstance = this;
    _x = 0; _y = 0; _width = 0; _height = 0;
    // position(0, 0);
    // size(PushLib::DISPLAY_WIDTH, PushLib::DISPLAY_HEIGHT);
    _puictx._pctx = pctx;
    _puictx._rootWidget = this;

    _padLayoutWidget = std::make_unique<PadLayoutWidget>(this, &_puictx);
    _gridWidget = std::make_unique<GridWidget>(this, &_puictx);
    _moduleWidget = std::make_unique<ModuleWidget>(this, &_puictx);
    _browserWidget = std::make_unique<BrowserWidget>(this, &_puictx);

    switchToView(PushView::Grid);

    PushLib::ButtonColor play;
    play.btn = PushLib::Button::Play;
    play.color = playColor;
    play.anim = PushLib::LedAnimation();
    pctx->setButtonColor(play);

}

RootWidget::~RootWidget() {

}

PushLib::BoundingBox RootWidget::bounds() { //return BoundingBox of area that has to be redrawn
    PushLib::BoundingBox b;
    if(_viewSwitched) {
        b.x = 0;
        b.y = 0;
        b.h = PushLib::DISPLAY_HEIGHT;
        b.w = PushLib::DISPLAY_WIDTH;
        _viewSwitched = false;
    } else {
        //invalidate?
        // b = widgetFromView(_currentView)->bounds();
        _dirtyRegions.clear();
        collectDirty(widgetFromView(currentView()), _dirtyRegions);
        for(auto &r : _dirtyRegions)
            b.unionWith(r);
    }
    return b;
}

void RootWidget::paint(PushLib::Painter &painter) {
    _dirtyRegions.clear();
            
    PushLib::Widget * toUpdate = widgetFromView(currentView());
    collectDirty(toUpdate, _dirtyRegions);

    if(_dirtyRegions.empty()) return;

    for(const PushLib::BoundingBox &b : _dirtyRegions) {
        traverse(toUpdate, painter, b);
    }

    finalize(toUpdate);
}

bool RootWidget::handleButton(PushLib::ButtonEvent &ev) {
    return widgetFromView(currentView())->handleButton(ev);
}

bool RootWidget::handleEncoder(PushLib::EncoderEvent &ev) {
    // std::string text;
    // if(ev.type == PushLib::EncoderEventType::Touched) text = "Touched";
    // else if(ev.type == PushLib::EncoderEventType::Moved) text = "Moved";
    // else if(ev.type == PushLib::EncoderEventType::Released) text = "Released";
    // LOG_WARN("Encoder Event: %s %d %d", text.c_str(), static_cast<int>(ev.encoder), ev.delta);
    return widgetFromView(_currentView)->handleEncoder(ev);
}

PushLib::Widget * RootWidget::widgetFromView(const PushView view) {
    Widget * ret = nullptr;
    switch(view) {
        case(PushView::ERROR): LOG_ERROR("ooops, error"); break;
        case(PushView::Grid): ret = _gridWidget.get(); break;
        case(PushView::Module): ret = _moduleWidget.get(); break;
        case(PushView::Editor): break;
        case(PushView::Patch): break;
        case(PushView::StepSequencer): break;
        case(PushView::ModMatrix): break;
        case(PushView::Browser): ret = _browserWidget.get(); break;
        case(PushView::Metronome): break;
        case(PushView::ScaleSelector): ret = _padLayoutWidget.get(); break;
        case(PushView::PushInternalSettings): break;
        case(PushView::SLRSettings): break;
    }
 
    return ret;
}

void RootWidget::switchToView(PushView view) {
    if(view == _currentView) {
        switchToView(_previousView);
        return;
    }

    _previousView = _currentView;
    _currentView = view;
    _viewSwitched = true;
    widgetFromView(view)->markAllDirty();
    colorButtons();
}

void RootWidget::goToPreviousView() {
    switchToView(_previousView);
}

const PushView RootWidget::previousView() const {
    return _previousView;
}

const PushView RootWidget::currentView() const {
    return _currentView;
}

bool RootWidget::handleDefaultButton(PushLib::ButtonEvent &ev) {
    //use switch later...
    const auto res = _buttonsCallback.find(ev.button);
    if(res != _buttonsCallback.end()) {
        const auto cb = res->second;
        return (this->*cb)(ev);
    }

    return false;
}

bool RootWidget::handleDefaultEncoder(PushLib::EncoderEvent &ev) {
    return false;
}

std::vector<PushLib::ButtonColor> RootWidget::buttonsColors() {
    return PushHelper::buttonColorsFromMap<RootWidget>(RootWidget::_buttonsCallback);
}

bool RootWidget::checkForRedraw() {
    /* 
    //when audiounitview parameter changed(that means it has changed in rt as well) -> mark unit as dirty...
    
    run through current view and check if this view need for redraw something
    e.g. if current view is module Track and we changed volume on track -> tracks audio unit view marked dirty -> we check that 
    trackUI->_unitView->dirty() ? return true : return false;

    if any of something requires redraw than return true -> push will call all things to redraw, otherwise nothing to redraw...
    */

    return false;
}

void RootWidget::createUI(const slr::Module * mod, slr::AudioUnitView * view) {
    // UnitUIBase * base = mod->createUI(view, &_uiContext);
    // base->create(&_uiContext);
    // _uiContext._unitsUI.push_back(base);
    LOG_INFO("Push create UI for %s", mod->_name->data());
}

void RootWidget::updateUI(slr::ID id) {
    // UnitUIBase * ui = nullptr;
    // for(UnitUIBase * u : _uiContext._unitsUI) {
    //     if(u->id() == id) {
    //         ui = u;
    //         break;
    //     }
    // }

    // if(!ui) {
    //     LOG_ERROR("Failed to find UI with id %u", id);
    //     return;
    // }

    // ui->update(&_uiContext);
    LOG_INFO("Push update UI for id %d", id);
}

void RootWidget::destroyUI(slr::ID id) {
    // UnitUIBase * ui = nullptr;
    // for(UnitUIBase * u : _uiContext._unitsUI) {
    //     if(u->id() == id) {
    //         ui = u;
    //         break;
    //     }
    // }

    // if(!ui) {
    //     LOG_ERROR("Failed to find UI with id %u", id);
    //     return;
    // }


    // std::size_t size = _uiContext._unitsUI.size();
    // std::size_t idx = 0;
    // for(std::size_t i=0; i<size; ++i) {
    //     if(_uiContext._unitsUI.at(i)->id() == id) {
    //         // found = _trackGuiList.at(i).get();
    //         idx = i;
    //         break;
    //     }
    // }

    // _uiContext._unitsUI.erase(_uiContext._unitsUI.begin()+idx);
    // //move items positions up starting from idx 
    // size -= 1;
    // for(std::size_t i=0; i<size; ++i) {
    //     UnitUIBase * tr = _uiContext._unitsUI.at(i);
    //     int x = 0;
    //     int y = LayoutDef::calcTrackY(i);
    //     // int x = tr->getPosX();
    //     // int y = tr->getPosY();
    //     tr->updatePosition(x, y);
    // }

    // _uiContext.setLastSelected(nullptr);
    // ui->destroy(&_uiContext);
    // delete ui;
    LOG_INFO("Push destroy UI for id %d", id);
}


RootWidget * RootWidget::inst() {
    return _rootInstance;
}



void RootWidget::colorButtons() {
    // _puictx.pctx()->clearButtonColors();
    // std::vector<PushLib::ButtonColor> map = PushHelper::buttonColorsFromMap<RootWidget>(RootWidget::_buttonsCallback);
    // _puictx.pctx()->setButtonsColors(map);
    //1. get from current view
    //2. get main
    //3. make diff where
    //  base is from view
    //  rest is from main

    std::vector<PushLib::ButtonColor> viewMap = widgetFromView(currentView())->buttonsColors();
    std::vector<PushLib::ButtonColor> defaultMap = _colors;

    for(auto &v : defaultMap) {
        if(v.btn != PushLib::Button::Play) continue;

        v.color = 15;
        break;
    }

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

bool RootWidget::userBtnClb(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;
    
    switchToView(PushView::Grid);
    return true;
}

bool RootWidget::deviceBtnClb(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;
    
    switchToView(PushView::Module);
    return true;
}

bool RootWidget::browserBtnClb(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;
    
    switchToView(PushView::Browser);
    return true;
}

bool RootWidget::scaleSelector(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;
    
    switchToView(PushView::ScaleSelector);
    return true;
}

bool RootWidget::layoutSelector(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;
    
    switchToView(PushView::ScaleSelector);
    return true;
}

bool RootWidget::playButtonClb(PushLib::ButtonEvent &ev) {
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

bool RootWidget::recordButtonClb(PushLib::ButtonEvent &ev) {
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