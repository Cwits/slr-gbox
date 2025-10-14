// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "lvgl.h"
#include "ui/display/primitives/BaseWidget.h"
#include "ui/display/primitives/DragContext.h"
#include "ui/display/primitives/UIContext.h"
#include "ui/display/PopupManager.h"

#include "defines.h"

namespace slr {
    class AudioUnitView;
    class Module;
}

namespace UI {

class UnitUIBase;
class View;

class TopPanel;
class BottomPanel;
class GridView;
class ModuleView;
class Browser;

class Popup;
class UnitControlPopup;
class RouteManager;
class TimelinePopup;
class ScreenKeyboard;
class FilePopup;
class NewModulePopup;

struct MainWindow : public BaseWidget {
    MainWindow(lv_obj_t * screen);
    ~MainWindow();
    
    void switchToView(MainView view);
    MainView previousView() const { return _prevView; }
    void switchToPreviousView();

    bool handleGesture(GestLib::Gesture & gesture);
    void transferGesture(MainView view, GestLib::Gestures gesture);

    void floatingTextRegular(std::string text);
    void floatingTextWarning(std::string text);

    void updateTimeline(const bool timeSigOrBpm);
    void updatePlayheadPosition(slr::frame_t position);

    void updateMetronomeState(bool onoff);
    
    void clearHittestTarget() { _initialGestureTarget = nullptr; }
    bool cancleGesture(BaseWidget * widget);

    void createUI(const slr::Module * mod, slr::AudioUnitView * view);
    void updateUI(slr::ID id);
    void destroyUI(slr::ID id);

    // void setLastSelected(UnitUIBase * unit);
    // UnitUIBase * lastSelectedModule() const { return _lastSelectedModule; }
    
    static MainWindow * inst();
    
    TopPanel * _topPanel;
    BottomPanel * _bottomPanel;

    //Main panels
    GridView * _gridView;
    ModuleView * _moduleView; //TODO: last selected unit view
    Browser * _browser;;

    //popups
    UnitControlPopup * _unitControlPopup;
    RouteManager * _routeManager;
    TimelinePopup * _timelinePopup;
    ScreenKeyboard * _keyboard;
    FilePopup * _filePopup;
    NewModulePopup * _newModulePopup;
    // ViewSelector * _viewSelector; //called only when need to switch from drag to target?
    //Slider mode popup * //aka AKAI MPC Live 3

    private:
    UIContext _uiContext;
    PopupManager _popManager;

    std::vector<Popup*> _popups; //only one popup must be active at the time
    
    MainView _currentView;
    MainView _prevView;

    BaseWidget * _gestureTarget;
    BaseWidget * _initialGestureTarget;
    BaseWidget * hitTest(BaseWidget * node, int x, int y);


    lv_obj_t * _floatingText;
    lv_timer_t * _floatingTimer;
    static void timercb(lv_timer_t * timer);

    View * getSwitchViewTarget(MainView & view);

    lv_timer_t * _playheadUpdateTimer;
    static void playheadUpdateCb(lv_timer_t * timer);

    UnitUIBase * _lastSelectedModule = nullptr;
};

}