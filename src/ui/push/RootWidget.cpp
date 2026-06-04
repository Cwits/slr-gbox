// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/push/RootWidget.h"

#include "ui/push/PadLayoutWidget.h"
#include "ui/push/GridWidget.h"
#include "ui/push/UnitWidget.h"
#include "ui/push/BrowserWidget.h"
#include "ui/push/primitives/UnitUIBase.h"

#include "push/PushContext.h"
#include "push/PushPainter.h"
#include "push/helper.h"


#include "snapshots/AudioUnitView.h"
#include "core/UnitManager.h"
// #include "core/Events.h"

#include "logger.h"

#include <string>
#include <algorithm>
#include <vector>

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

std::vector<std::unique_ptr<UnitUIBase>> _removedUnits;

std::vector<PushLib::BoundingBox> _dirtyRegions;

void collectDirty(PushLib::Widget *w, std::vector<PushLib::BoundingBox> &list) {
    if(!w->visible()) return; 

    if(w->dirty()) {
        auto cur = w->globalBounds();
        auto old = w->lastGlobalBounds();

        if(!cur.empty()) list.push_back(cur);
        if(!old.empty()) {
            if(old != cur)
                list.push_back(old);
        }
    }

    for(PushLib::Widget *c : w->childs())
        collectDirty(c, list);
};


void traverse(PushLib::Widget *w, PushLib::Painter &p, const PushLib::BoundingBox &clip) {
    if(!w->visible()) return;

    auto global = w->globalBounds();
    // if(!global.intersects(clip)) return;

    PushLib::BoundingBox newClip = global.intersect(clip);
    if(newClip.empty()) return;

    p.pushClip(newClip);
    p.pushOffset(global.x, global.y);
    
    w->paint(p);
    
    w->sortChildsByZ();
    for(PushLib::Widget *c : w->childs()) 
        traverse(c, p, newClip);
    
    p.popOffset();
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
    // _x = 0; _y = 0; _width = 0; _height = 0;
    position(0, 0);
    size(PushLib::DISPLAY_WIDTH, PushLib::DISPLAY_HEIGHT);
    _puictx._pctx = pctx;
    _puictx._rootWidget = this;

    _padLayoutWidget = std::make_unique<PadLayoutWidget>(this, &_puictx);
    _gridWidget = std::make_unique<GridWidget>(this, &_puictx);
    _unitWidget = std::make_unique<UnitWidget>(this, &_puictx);
    _browserWidget = std::make_unique<BrowserWidget>(this, &_puictx);

    _puictx._gridWidget = _gridWidget.get();
    _puictx._unitWidget = _unitWidget.get();

    switchToView(PushView::Grid);

    PushLib::ButtonColor play;
    play.btn = PushLib::Button::Play;
    play.color = playColor;
    play.anim = PushLib::LedAnimation();
    pctx->setButtonColor(play);

}

RootWidget::~RootWidget() {

}

void RootWidget::paint(PushLib::Painter &painter) {
    _dirtyRegions.clear();
            
    PushLib::Widget * toUpdate = widgetFromView(currentView());
    collectDirty(toUpdate, _dirtyRegions);

    if(_dirtyRegions.empty()) { clearDirty(); return; }

    for(const PushLib::BoundingBox &b : _dirtyRegions) {
        traverse(toUpdate, painter, b);
    }

    finalize(toUpdate);
    clearDirty();
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

PushLib::Widget * RootWidget::widgetFromView(const PushView view) const {
    Widget * ret = nullptr;
    switch(view) {
        case(PushView::ERROR): LOG_ERROR("ooops, error"); break;
        case(PushView::Grid): ret = _gridWidget.get(); break;
        case(PushView::Unit): ret = _unitWidget.get(); break;
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

void RootWidget::createUI(const slr::UnitDescriptor *desc, const std::shared_ptr<const slr::AudioUnitView> &view) {
    std::unique_ptr<UnitUIBase> unitUI = desc->createPushUI(view, &_puictx);
    unitUI->create(&_puictx);

    _puictx._unitUIs.push_back(std::move(unitUI));

    // _puictx.forceRedraw();
    // LOG_INFO("Push create UI for %s", mod->_name->data());
}

void RootWidget::restoreUI(slr::ID id) {
    auto it = std::find_if(
        _puictx._unitUIs.begin(),
        _puictx._unitUIs.end(),
        [id](const auto & ui) {
            return id == ui->id();
        }
    );

    if(it == _puictx._unitUIs.end()) {
        LOG_ERROR("Failed to find such UI for id %u", id);
        return;
    }

    UnitUIBase * ptr = (*it).get();
    _puictx._unitUIs.push_back(std::move(*it));

    int y = 0;
    for(auto &ui : _puictx._unitUIs) {
        DefaultGridUI * grid = ui->gridUI();
        PushLib::Vec2 oldPos = grid->position();
        int newy = (70 * y) + (5*y) + 13;
        PushLib::Vec2 newPos = PushLib::Vec2(oldPos.x(), newy);
        grid->position(newPos);
        y++;
    }

    ptr->show();
    // _uiContext.setLastSelected(nullptr);
}

void RootWidget::removeUI(slr::ID id) {
    auto it = std::find_if(
            _puictx._unitUIs.begin(),
            _puictx._unitUIs.end(),
            [id](const auto & ui) {
                return id == ui->id();
            }
    );

    if(it == _puictx._unitUIs.end()) {
        LOG_ERROR("Failed to find such UI for id %u", id);
        return;
    }
    
    (*it)->hide();
    _removedUnits.push_back(std::move(*it));
    _puictx._unitUIs.erase(it);

    int y = 0;
    for(auto &ui : _puictx._unitUIs) {
        DefaultGridUI * grid = ui->gridUI();
        PushLib::Vec2 oldPos = grid->position();
        int newy = (70 * y) + (5*y) + 13;
        PushLib::Vec2 newPos = PushLib::Vec2(oldPos.x(), newy);
        grid->position(newPos);
        y++;
    }

    // LOG_INFO("Push destroy UI for id %d", id);
}

void RootWidget::deleteUI(slr::ID id) {
    // base->destroy(&_puictx);
    auto it = std::find_if(
            _puictx._unitUIs.begin(),
            _puictx._unitUIs.end(),
            [id](const auto & ui) {
                return id == ui->id();
            }
    );

    if(it == _puictx._unitUIs.end()) {
        LOG_ERROR("Failed to find UI to delete id %u", id);
        return;
    }
    
    (*it)->destroy(&_puictx);
    _removedUnits.erase(it);
}

void RootWidget::clearUI() {
    _puictx._unitUIs.clear();
}

bool RootWidget::hasAnythingDirty() const {
    return widgetFromView(currentView())->hasAnythingDirty();
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
    
    switchToView(PushView::Unit);
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