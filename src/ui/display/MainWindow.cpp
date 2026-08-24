// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/MainWindow.h"
#include "ui/display/layoutSizes.h"
#include "ui/display/defaultStyles.h"
#include "ui/display/defaultColors.h"
#include "ui/display/primitives/FileView.h"
#include "ui/display/primitives/UnitUIBase.h"
#include "ui/display/primitives/UIContext.h"
#include "ui/display/primitives/Label.h"
#include "ui/display/primitives/DragContext.h"

#include "ui/display/GridView.h"
#include "ui/display/UnitView.h"
#include "ui/display/Browser.h"
#include "ui/display/TopPanel.h"
#include "ui/display/BottomPanel.h"
#include "ui/display/StepSequencerView.h"
#include "ui/display/ModEngineView.h"

#include "ui/display/RouteManager.h"
#include "ui/display/TimelinePopup.h"
#include "ui/display/ScreenKeyboard.h"
#include "ui/display/NewUnitPopup.h"
#include "ui/display/SettingsPopup.h"
#include "ui/display/VirtualMidiKeyboard.h"
#include "ui/display/Timeline.h"
#include "ui/display/DragViewSelector.h"

#include "snapshots/TimelineView.h"

#include "core/UnitManager.h"
#include "core/Actions.h"

#include "logger.h"

#include <iostream>
#include <algorithm>
#include <vector>
#include <memory>

namespace UI {

constexpr int FLOATING_TEXT_TIMEOUT = 5000;

MainWindow * _inst = nullptr;

std::vector<std::unique_ptr<UnitUIBase>> _removedUnits;

std::string gestureToText(GestLib::Gestures &g);

MainWindow::MainWindow(lv_obj_t * screen) : BaseWidget(screen) {
    _inst = this;
    initDefaultStyles();
    lv_obj_set_style_bg_color(_lvhost, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
    
    // _uiContext = new UIContext();
    _uiContext._mainWindow = this;

    _topPanel = std::make_unique<TopPanel>(this, &_uiContext);
    _bottomPanel = std::make_unique<BottomPanel>(this, &_uiContext);
    _gridView = std::make_unique<GridView>(this, &_uiContext);
    _unitView = std::make_unique<UnitView>(this, &_uiContext);
    _browser = std::make_unique<Browser>(this, &_uiContext);
    _stepSequencerView = std::make_unique<StepSequencerView>(this, &_uiContext);
    _modEngineView = std::make_unique<ModEngineView>(this, &_uiContext);
    
    //TODO: set context values...
    _uiContext._topPanel = _topPanel.get();
    _uiContext._bottomPanel = _bottomPanel.get();
    _uiContext._gridView = _gridView.get();
    _uiContext._unitView = _unitView.get();
    _uiContext._browser = _browser.get();
    _uiContext._stepSequencerView = _stepSequencerView.get();
    _uiContext._gridTimeline = _gridView->_timeline.get();
    
    _dragViewSelector = std::make_unique<DragViewSelector>(this, &_uiContext);
    _uiContext._dragSelector = _dragViewSelector.get();

    _playheadUpdateTimer = lv_timer_create(&MainWindow::playheadUpdateCb, LV_DEF_REFR_PERIOD, nullptr);
    lv_timer_pause(_playheadUpdateTimer);
    lv_timer_set_auto_delete(_playheadUpdateTimer, false);

    _gestureTarget = nullptr;
    _initialGestureTarget = nullptr;
    _currentView = MainView::Grid;
    _prevView = _currentView;
    
    // _uiContext._dragContext.reset();

    _unitControlPopup = std::make_unique<UnitControlPopup>(this, &_uiContext);
    _popups.push_back(_unitControlPopup.get());
    _routeManager = std::make_unique<RouteManager>(this, &_uiContext);
    _popups.push_back(_routeManager.get());
    _timelinePopup = std::make_unique<TimelinePopup>(this, &_uiContext);
    _popups.push_back(_timelinePopup.get());
    _keyboard = std::make_unique<ScreenKeyboard>(this, &_uiContext);
    _popups.push_back(_keyboard.get());
    _filePopup = std::make_unique<FilePopup>(this, &_uiContext);
    _popups.push_back(_filePopup.get());
    _newUnitPopup = std::make_unique<NewUnitPopup>(this, &_uiContext);
    _popups.push_back(_newUnitPopup.get());
    _settingsPopup = std::make_unique<SettingsPopup>(this, &_uiContext);
    _popups.push_back(_settingsPopup.get());
    _virtualMidiKeyboard = std::make_unique<VirtualMidiKeyboard>(this, &_uiContext);
    _popups.push_back(_virtualMidiKeyboard.get());
    // _dragViewSelector = std::make_unique<DragViewSelector>(this, &_uiContext);
    _popups.push_back(_dragViewSelector.get());
    _targetSelectPopup = std::make_unique<TargetSelectPopup>(this, _stepSequencerView.get(), &_uiContext);
    _popups.push_back(_targetSelectPopup.get());
    _stepSequencerView->_tpop = _targetSelectPopup.get();

    _popManager._unitControlPopup = _unitControlPopup.get();
    _popManager._routeManager = _routeManager.get();
    _popManager._timelinePopup = _timelinePopup.get();
    _popManager._screenKeyboard = _keyboard.get();
    _popManager._filePopup = _filePopup.get();
    _popManager._newUnitPopup = _newUnitPopup.get();
    _popManager._settingsPopup = _settingsPopup.get();
    _popManager._virtualMidiKeyboard = _virtualMidiKeyboard.get();
    _popManager._dragViewSelector = _dragViewSelector.get();
    _popManager._targetSelectPopup = _targetSelectPopup.get();

    _uiContext._popManager = &_popManager;

    /* let it be last one because of order of drawing */
    _floatingText = lv_label_create(_lvhost);
    lv_obj_set_style_text_font(_floatingText, &lv_font_montserrat_40, 0);
    lv_obj_set_pos(_floatingText, 30, LayoutDef::TOTAL_HEIGHT-200);
    _floatingTimer = lv_timer_create(&MainWindow::floatingTimercb, FLOATING_TEXT_TIMEOUT, _floatingText);
    lv_timer_set_auto_delete(_floatingTimer, false);

    // lv_timer_set_repeat_count(_floatingTimer, 1);
    lv_obj_add_flag(_floatingText, LV_OBJ_FLAG_HIDDEN);

    _dragContext = std::make_unique<DragContext>(this);
    _dragContext->reset();
    _uiContext._dragContext = _dragContext.get();
}

MainWindow::~MainWindow() {
    //but no clean of lvgl object - would be cleaned on main exit
    for(std::unique_ptr<UnitUIBase> &b : _uiContext._unitsUI) {
        b->destroy(&_uiContext);
    }

    for(std::unique_ptr<UnitUIBase> &b : _removedUnits) {
        b->destroy(&_uiContext);
    }
}

void MainWindow::switchToView(MainView view) {
    _gridView->hide();
    _unitView->hide();
    _browser->hide();
    _stepSequencerView->hide();
    _modEngineView->hide();

    View * target = getSwitchViewTarget(view);
    target->update();
    target->show();

    _prevView = _currentView;
    _currentView = view;
}

void MainWindow::switchToPreviousView() {
    if(_prevView != _currentView) {
        switchToView(_prevView);
    }
}

bool MainWindow::cancleGesture(BaseWidget * widget) {
    if(_gestureTarget == widget) {
        _gestureTarget = nullptr;
        _initialGestureTarget = nullptr;
        return true;
    }

    LOG_WARN("Failed to cancle gesture");
    return false;
}

bool MainWindow::handleGesture(GestLib::Gesture & gesture) {
    BaseWidget * globtarget = nullptr;
    if(gesture.type == GestLib::Gestures::TouchDown) {
        int x = gesture.touchDown.x;
        int y = gesture.touchDown.y;

        BaseWidget * node = nullptr;
        
        Popup * pop = nullptr;
        bool popActive = false;
        for(Popup * p : _popups) {
            if(p->active()) {
                pop = p;
                popActive = true;
                break;
            }
        }

        if(popActive) {
            node = pop;
        } else {

            if(y <= LayoutDef::TOP_PANEL_HEIGHT) {
                //look in top panel
                node = _topPanel.get();
            } else if(y > LayoutDef::TOP_PANEL_HEIGHT && y < (LayoutDef::WORKSPACE_HEIGHT+LayoutDef::TOP_PANEL_HEIGHT)) {
                //look in workspace
                node = getSwitchViewTarget(_currentView);
            } else {
                //look in bottom panel
                //node = _bottomPanel;
                node = _bottomPanel.get();
            }

        }

        BaseWidget * target = hitTest(node, x, y);
        
        if(target == nullptr && popActive) {
            pop->hide();
            pop->deactivate();
            return false;
        }
        
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
        // if(!_gestureTarget->canHandleGesture(gesture.type)) {
        //     BaseWidget * node = _gestureTarget->parent();
        //     while(node) {
        //         if(node->canHandleGesture(gesture.type)) {
        //             _gestureTarget = node;
        //             break;
        //         }
        //         node = node->parent();
        //         if(node == nullptr) 
        //             return false;
        //     }
        // }

        // bool ret =_gestureTarget->handleGesture(gesture);
        // return ret;
    }

    if(!globtarget) {
        LOG_WARN("Hittest target is nullptr in %s gesture", gestureToText(gesture.type).c_str());
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


BaseWidget * MainWindow::hitTest(BaseWidget * node, int x, int y) {
    static auto zsort = [](const BaseWidget *op1, const BaseWidget *op2) -> bool {
        return (op1->getZ() < op2->getZ());
    };

    std::vector<BaseWidget*> sorted;
    sorted = node->children();
    std::sort(sorted.begin(), sorted.end(), zsort);

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


//transfers ongoing gesture to different view(e.g. from browser to grid)
void MainWindow::transferGesture(BaseWidget * target, GestLib::Gestures gesture) {
    if(!target->canHandleGesture(gesture)) {
        LOG_WARN("Target can't handle gesture %s", gestureToText(gesture).c_str());
        return;
    }

    if(target == _dragViewSelector.get()) {
        _gestureTarget = _dragViewSelector.get();
    } else if(target == _gridView.get()) {
        _gestureTarget = _gridView.get();
    } else if(target == _browser.get()) {
        _gestureTarget = _browser.get();
    } else if(target == _unitView.get()) {
        _gestureTarget = _unitView.get();
    } else if(target == _modEngineView.get()) {
        _gestureTarget = _modEngineView.get();
    } else {
        LOG_ERROR("Target not handled");
    }
}

void MainWindow::floatingText(bool warning, const std::string &text) {
    if(warning) {
        LOG_WARN("%s", text.c_str());
        lv_obj_set_style_text_color(_floatingText, FLOATING_TEXT_WARNING_COLOR, 0);    
    } else {
        LOG_INFO("%s", text.c_str());
        lv_obj_set_style_text_color(_floatingText, FLOATING_TEXT_REGULAR_COLOR, 0);
    }
    lv_label_set_text(_floatingText, text.c_str());
    lv_obj_move_to_index(_floatingText, -1);
    lv_obj_clear_flag(_floatingText, LV_OBJ_FLAG_HIDDEN);
    lv_timer_reset(_floatingTimer);
    lv_timer_resume(_floatingTimer);
}

void MainWindow::floatingTimercb(lv_timer_t * timer) {
    lv_obj_t * label = static_cast<lv_obj_t*>(lv_timer_get_user_data(timer));
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    lv_timer_pause(timer);
}

MainWindow * MainWindow::inst() {
    return _inst;
}

View * MainWindow::getSwitchViewTarget(MainView & view) {
    View * ret = nullptr;
    switch(view) {
        case(MainView::Grid): ret = _gridView.get(); break;
        case(MainView::Unit): ret = _unitView.get(); break;
        case(MainView::Browser): ret = _browser.get(); break;
        case(MainView::Patch): ; break;
        case(MainView::Editor): ; break;
        case(MainView::StepSequencer): ret = _stepSequencerView.get(); break;
        case(MainView::ModEngine): ret = _modEngineView.get(); break;
    }
    return ret;
}

void MainWindow::playheadUpdateCb(lv_timer_t * timer) {
    auto act = std::make_unique<slr::Actions::UpdatePlayhead>();
    slr::EmitAction(std::move(act));
    lv_timer_reset(timer);
}

void MainWindow::updateTimeline(const bool timeSigOrBpm) {
    _bottomPanel->updateTimelineRelated(timeSigOrBpm);

    slr::TimelineView &tl = slr::TimelineView::getTimelineView();
    switch(tl.state()) {
        case(slr::Timeline::RollState::Play): {
            lv_timer_resume(_playheadUpdateTimer);
        } break;
        case(slr::Timeline::RollState::Pause): {
            lv_timer_pause(_playheadUpdateTimer);
        } break;
        case(slr::Timeline::RollState::Stop): {
            lv_timer_pause(_playheadUpdateTimer);
        }
        case(slr::Timeline::RollState::Preparing): break;
    }

    if(timeSigOrBpm) {
        // LOG_WARN("Recalculate timeline grid");
        //recalculate grid
        _gridView->_timeline->update();
    }
    
    _gridView->_timeline->updateLoopMarkers();
    _gridView->_timeline->showLoopMarkers(tl.looping());
}

void MainWindow::updatePlayheadPosition(slr::frame_t position) {
    _bottomPanel->_lblTestPlayhead->setText(std::to_string(position));
    _gridView->_timeline->updatePlayhead(position);
    _modEngineView->updateLine(position);
}

void MainWindow::updateMetronomeState(bool onoff) {
    if(onoff) {
        _topPanel->setMetroColor(METRONOME_ON_COLOR);
    } else {
        _topPanel->setMetroColor(BUTTON_DEFAULT_COLOR);
    }
}

void MainWindow::createSequenceUI(const std::shared_ptr<slr::SequenceView> view) {
    _stepSequencerView->createSequenceUI(view);
}

std::string gestureToText(GestLib::Gestures &g) {
    std::string text;
    switch(g) {
        case(GestLib::Gestures::TouchDown): text = "Touch Down"; break;
        case(GestLib::Gestures::TouchUp): text = "Touch Up"; break;
        case(GestLib::Gestures::Tap): text = "Tap"; break;
        case(GestLib::Gestures::Hold): text = "Hold"; break;
        case(GestLib::Gestures::Drag): text = "Drag"; break;
        case(GestLib::Gestures::Swipe): text = "Swipe"; break;
        case(GestLib::Gestures::DoubleTap): text = "Double Tap"; break;
        case(GestLib::Gestures::DoubleTapSwipe): text = "Double Tap Swipe"; break;
        case(GestLib::Gestures::DoubleTapCircular): text = "Double Tap Circular"; break;
        case(GestLib::Gestures::Zoom): text = "Zoom"; break;
        case(GestLib::Gestures::TwoFingerTap): text = "Two Finger Tap"; break;
        case(GestLib::Gestures::TwoFingerSwipe): text = "Two Finger Swipe"; break;
        case(GestLib::Gestures::ThreeFingerTap): text = "Three Finger Tap"; break;
        case(GestLib::Gestures::ThreeFingerSwipe): text = "Three Finger Swipe"; break;
    }
    return text;
}

void MainWindow::createUI(const slr::UnitDescriptor * desc, const std::shared_ptr<const slr::AudioUnitView> &view) {
    std::unique_ptr<UnitUIBase> base = desc->createUI(view, &_uiContext);
    base->create(&_uiContext);
    _uiContext._unitsUI.push_back(std::move(base));
}

void MainWindow::restoreUI(slr::ID id) {
    // UnitUIBase * ptr = ui.get();
    auto it = std::find_if(
        _removedUnits.begin(),
        _removedUnits.end(),
        [id](std::unique_ptr<UnitUIBase> &b) {
            return id == b->id();
        }
    );

    if(it == _removedUnits.end()) {
        LOG_ERROR("Failed to restore unit ui for id %u", id);
        return;
    }

    UnitUIBase *ptr = (*it).get();
    _uiContext._unitsUI.push_back(std::move(*it));
    _removedUnits.erase(it);

    std::size_t size = _uiContext._unitsUI.size();
    for(std::size_t i=0; i<size; ++i) {
        UnitUIBase * tr = _uiContext._unitsUI.at(i).get();
        int x = 0;
        int y = LayoutDef::calcTrackY(i);
        tr->gridUI()->updatePosition(x, y);
    }

    ptr->show();
    _uiContext.setLastSelected(nullptr);
}

void MainWindow::removeUI(slr::ID id) {
    auto it = std::find_if(
        _uiContext._unitsUI.begin(),
        _uiContext._unitsUI.end(),
        [id](const std::unique_ptr<UnitUIBase> &b) {
            return b->id() == id;
        }
    );
    if(it == _uiContext._unitsUI.end()) {
        LOG_ERROR("Failed to find UI with id %u", id);
        return;
    }

    (*it)->hide();
    _removedUnits.push_back(std::move(*it));
    // ui->hide();
    // ui->destroy(&_uiContext);
    _uiContext._unitsUI.erase(it);

    //move items positions up starting from idx 
    std::size_t size = _uiContext._unitsUI.size();
    for(std::size_t i=0; i<size; ++i) {
        UnitUIBase * tr = _uiContext._unitsUI.at(i).get();
        int x = 0;
        int y = LayoutDef::calcTrackY(i);
        tr->gridUI()->updatePosition(x, y);
    }

    _uiContext.setLastSelected(nullptr);
}

void MainWindow::deleteUI(slr::ID id) {
    // base->destroy(&_uiContext);
    auto it = std::find_if(
        _removedUnits.begin(),
        _removedUnits.end(),
        [id](std::unique_ptr<UnitUIBase> &b) {
            return id == b->id();
        }
    );

    if(it == _removedUnits.end()) {
        LOG_ERROR("Failed to delete unit ui for id %u", id);
        return;
    }

    (*it)->destroy(&_uiContext);
    _removedUnits.erase(it);
}

void MainWindow::clearUI() {
    _uiContext._unitsUI.clear();
}

void MainWindow::pollUIUpdate() {
    //depends on current view -> check updates?
    //check frequent updates e.g. animated, timeline or smth else
    
    // for(auto clb : _frequentUpdateCallbacks) { /// hmm... not the best option i guess?
    //     clb();
    // }

    //slow updates - roughly  30/15 ~= 2hz
    // static unsigned char slowdown = 0;
    // slowdown++;
    // if(slowdown < 15) return;
    // slowdown = 0;

    MainView view = currentView();
    getSwitchViewTarget(view)->pollUIUpdate();
}

void MainWindow::registerFrequentUpdate(std::function<void()> clb) {
    // _frequentUpdateCallbacks.push_back(std::move(clb));
}

} //namespace UI
