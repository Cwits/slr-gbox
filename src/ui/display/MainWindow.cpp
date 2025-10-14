// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/MainWindow.h"
#include "ui/display/layoutSizes.h"
#include "ui/display/defaultStyles.h"
#include "ui/display/defaultColors.h"
#include "ui/display/primitives/FileView.h"
#include "ui/display/primitives/UnitUIBase.h"
#include "ui/display/primitives/UIContext.h"

#include "ui/display/GridView.h"
#include "ui/display/ModuleView.h"
#include "ui/display/Browser.h"
#include "ui/display/TopPanel.h"
#include "ui/display/BottomPanel.h"

#include "ui/display/RouteManager.h"
#include "ui/display/TimelinePopup.h"
#include "ui/display/ScreenKeyboard.h"
#include "ui/display/NewModulePopup.h"

#include "snapshots/TimelineView.h"

#include "core/ModuleManager.h"
#include "core/Events.h"

#include "logger.h"

#include <iostream>

namespace UI {

constexpr int FLOATING_TEXT_TIMEOUT = 5000;

MainWindow * _inst = nullptr;

std::string gestureToText(GestLib::Gestures &g);

MainWindow::MainWindow(lv_obj_t * screen) : BaseWidget(screen) {
    _inst = this;
    initDefaultStyles();
    lv_obj_set_style_bg_color(_lvhost, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
    
    // _uiContext = new UIContext();
    _uiContext._mainWindow = this;

    _topPanel = new TopPanel(this, &_uiContext);
    _bottomPanel = new BottomPanel(this, &_uiContext);
    _gridView = new GridView(this, &_uiContext);
    _moduleView = new ModuleView(this, &_uiContext);
    _browser = new Browser(this, &_uiContext);
    
    //TODO: set context values...
    _uiContext._topPanel = _topPanel;
    _uiContext._bottomPanel = _bottomPanel;
    _uiContext._gridView = _gridView;
    _uiContext._module = _moduleView;
    _uiContext._browser = _browser;
    _uiContext._gridTimeline = _gridView->_timeline;

    _playheadUpdateTimer = lv_timer_create(&MainWindow::playheadUpdateCb, LV_DEF_REFR_PERIOD, nullptr);
    lv_timer_pause(_playheadUpdateTimer);
    lv_timer_set_auto_delete(_playheadUpdateTimer, false);

    _gestureTarget = nullptr;
    _initialGestureTarget = nullptr;
    _currentView = MainView::Grid;
    _prevView = _currentView;
    
    _uiContext._dragContext.reset();

    _unitControlPopup = new UnitControlPopup(this, &_uiContext);
    _popups.push_back(_unitControlPopup);
    _routeManager = new RouteManager(this, &_uiContext);
    _popups.push_back(_routeManager);
    _timelinePopup = new TimelinePopup(this, &_uiContext);
    _popups.push_back(_timelinePopup);
    _keyboard = new ScreenKeyboard(this, &_uiContext);
    _popups.push_back(_keyboard);
    _filePopup = new FilePopup(this, &_uiContext);
    _popups.push_back(_filePopup);
    _newModulePopup = new NewModulePopup(this, &_uiContext);
    _popups.push_back(_newModulePopup);

    _popManager._unitControlPopup = _unitControlPopup;
    _popManager._routeManager = _routeManager;
    _popManager._timelinePopup = _timelinePopup;
    _popManager._screenKeyboard = _keyboard;
    _popManager._filePopup = _filePopup;
    _popManager._newModulePopup = _newModulePopup;

    _uiContext._popManager = &_popManager;

    /* let it be last one because of order of drawing */
    _floatingText = lv_label_create(_lvhost);
    lv_obj_set_style_text_font(_floatingText, &lv_font_montserrat_40, 0);
    lv_obj_set_pos(_floatingText, 30, LayoutDef::TOTAL_HEIGHT-200);
    _floatingTimer = lv_timer_create(&MainWindow::timercb, FLOATING_TEXT_TIMEOUT, _floatingText);
    lv_timer_set_auto_delete(_floatingTimer, false);

    // lv_timer_set_repeat_count(_floatingTimer, 1);
    lv_obj_add_flag(_floatingText, LV_OBJ_FLAG_HIDDEN);

}

MainWindow::~MainWindow() {
    //but no clean of lvgl object - would be cleaned on main exit
    // delete _btnAddTrack;
    delete _gridView;
    delete _moduleView;
    delete _browser;
    delete _bottomPanel;
    delete _topPanel;
    delete _unitControlPopup;
    delete _routeManager;
    delete _timelinePopup;
    delete _keyboard;
    delete _filePopup;
    delete _newModulePopup;
    // delete _uiContext;
}

void MainWindow::switchToView(MainView view) {
    _gridView->hide();
    _moduleView->hide();
    _browser->hide();

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
                node = _topPanel;
            } else if(y > LayoutDef::TOP_PANEL_HEIGHT && y < (LayoutDef::WORKSPACE_HEIGHT+LayoutDef::TOP_PANEL_HEIGHT)) {
                //look in workspace
                switch(_currentView) {
                    case(MainView::Grid): node = _gridView; break;
                    case(MainView::Module): node = _moduleView; break;
                    case(MainView::Browser): node = _browser; break;
                }

            } else {
                //look in bottom panel
                //node = _bottomPanel;
                node = _bottomPanel;
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
    // Go from front-most (last) to back-most (first)
    for(auto it = node->children().rbegin(); it != node->children().rend(); ++it) {
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
void MainWindow::transferGesture(MainView view, GestLib::Gestures gesture) {
    if(gesture == GestLib::Gestures::Drag) {
        //TODO: shouldn't be like that... dunno yet how to make it proper way
        /*
            в чем проблема - _gridView->_controls тоже будут поддерживать drag, но не для файлов, а для пресетов треков
            (хотя файлы тоже можно, просто будут помещены в начало)...
            и, надо как то на лету понимать потенциальную цель(допустим если расширение объекта - аудио или миди файл то на _grid надо,
            если же пресет то на _control пробрасывать). Просто вызвать getSwitchViewTarget(view) не получится
        */
        _gestureTarget = _gridView->_grid;
        // _gestureTarget = getSwitchViewTarget(view);
        switchToView(view);
    }
}

void MainWindow::floatingTextRegular(std::string text) {
    lv_obj_set_style_text_color(_floatingText, FLOATING_TEXT_REGULAR_COLOR, 0);
    lv_label_set_text(_floatingText, text.c_str());
    lv_obj_clear_flag(_floatingText, LV_OBJ_FLAG_HIDDEN);
    lv_timer_resume(_floatingTimer);
}

void MainWindow::floatingTextWarning(std::string text) {
    lv_obj_set_style_text_color(_floatingText, FLOATING_TEXT_WARNING_COLOR, 0);
    lv_label_set_text(_floatingText, text.c_str());
    lv_obj_clear_flag(_floatingText, LV_OBJ_FLAG_HIDDEN);
    lv_timer_resume(_floatingTimer);
}

void MainWindow::timercb(lv_timer_t * timer) {
    lv_obj_t * label = static_cast<lv_obj_t*>(lv_timer_get_user_data(timer));
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    lv_timer_pause(timer);
    lv_timer_reset(timer);
}

MainWindow * MainWindow::inst() {
    return _inst;
}

View * MainWindow::getSwitchViewTarget(MainView & view) {
    View * ret = nullptr;
    switch(view) {
        case(MainView::Grid): ret = _gridView; break;
        case(MainView::Module): ret = _moduleView; break;
        case(MainView::Browser): ret = _browser; break;
        case(MainView::Patch): ; break;
        case(MainView::Editor): ; break;
        case(MainView::StepSequencer): ; break;
        case(MainView::ModMatrix): ; break;
    }
    return ret;
}

void MainWindow::playheadUpdateCb(lv_timer_t * timer) {
    slr::Events::RequestPlayhead e;
    slr::EmitEvent(e);
    // lv_timer_reset(timer);
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
    _bottomPanel->_testPlayhead->setText(std::to_string(position));
    _gridView->_timeline->updatePlayhead(position);
}

void MainWindow::updateMetronomeState(bool onoff) {
    if(onoff) {
        _topPanel->setMetroColor(METRONOME_ON_COLOR);
    } else {
        _topPanel->setMetroColor(BUTTON_DEFAULT_COLOR);
    }
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

void MainWindow::createUI(const slr::Module * mod, slr::AudioUnitView * view) {
    UnitUIBase * base = mod->createUI(view, &_uiContext);
    base->create(&_uiContext);
    _uiContext._unitsUI.push_back(base);
}

void MainWindow::updateUI(slr::ID id) {
    UnitUIBase * ui = nullptr;
    for(UnitUIBase * u : _uiContext._unitsUI) {
        if(u->id() == id) {
            ui = u;
            break;
        }
    }

    if(!ui) {
        LOG_ERROR("Failed to find UI with id %u", id);
        return;
    }

    ui->update(&_uiContext);
}

void MainWindow::destroyUI(slr::ID id) {
    UnitUIBase * ui = nullptr;
    for(UnitUIBase * u : _uiContext._unitsUI) {
        if(u->id() == id) {
            ui = u;
            break;
        }
    }

    if(!ui) {
        LOG_ERROR("Failed to find UI with id %u", id);
        return;
    }


    std::size_t size = _uiContext._unitsUI.size();
    std::size_t idx = 0;
    for(std::size_t i=0; i<size; ++i) {
        if(_uiContext._unitsUI.at(i)->id() == id) {
            // found = _trackGuiList.at(i).get();
            idx = i;
            break;
        }
    }

    _uiContext._unitsUI.erase(_uiContext._unitsUI.begin()+idx);
    //move items positions up starting from idx 
    size -= 1;
    for(std::size_t i=0; i<size; ++i) {
        UnitUIBase * tr = _uiContext._unitsUI.at(i);
        int x = 0;
        int y = LayoutDef::calcTrackY(i);
        // int x = tr->getPosX();
        // int y = tr->getPosY();
        tr->updatePosition(x, y);
    }

    _uiContext.setLastSelected(nullptr);
    ui->destroy(&_uiContext);
    delete ui;
}

// void MainWindow::setLastSelected(UnitUIBase * unit) {
//     if(unit == nullptr) {
//         _lastSelectedModule = nullptr;
//         lv_obj_add_flag(_gridView->_control->_lastSelectedRect, LV_OBJ_FLAG_HIDDEN);
//     } else {
//         _lastSelectedModule = unit;
//     }

//     _moduleView->update();
//     // for(std::size_t i=0; i<_unitsUI.size(); ++i) {
//     //     _unitsUI.at(i)->moduleView()->hide();
//     // }
//     // unit->moduleView()->show();
// }

} //namespace UI
