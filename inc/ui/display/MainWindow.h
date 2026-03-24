// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "lvgl.h"
#include "ui/display/primitives/BaseWidget.h"
#include "ui/display/primitives/UIContext.h"
#include "ui/display/PopupManager.h"

#include "defines.h"

#include <memory>

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
class SettingsPopup;
class VirtualMidiKeyboard;

class DragContext;

struct MainWindow : public BaseWidget {
    MainWindow(lv_obj_t * screen);
    ~MainWindow();
    
    void switchToView(MainView view);
    MainView currentView() const { return _currentView; }
    MainView previousView() const { return _prevView; }
    void switchToPreviousView();

    bool handleGesture(GestLib::Gesture & gesture);
    void transferGesture(MainView view, GestLib::Gestures gesture);

    void floatingText(bool warning, const std::string &text);

    void updateTimeline(const bool timeSigOrBpm);
    void updatePlayheadPosition(slr::frame_t position);

    void updateMetronomeState(bool onoff);
    
    void clearHittestTarget() { _initialGestureTarget = nullptr; }
    bool cancleGesture(BaseWidget * widget);

    void createUI(const slr::Module * mod, slr::AudioUnitView * view);
    void destroyUI(slr::ID id);

    void pollUIUpdate() override;
    // void setLastSelected(UnitUIBase * unit);
    // UnitUIBase * lastSelectedModule() const { return _lastSelectedModule; }
    
    void registerFrequentUpdate(std::function<void()> clb);

    static MainWindow * inst();
    
    std::unique_ptr<TopPanel> _topPanel;
    std::unique_ptr<BottomPanel> _bottomPanel;

    //Main views
    std::unique_ptr<GridView> _gridView;
    std::unique_ptr<ModuleView> _moduleView; //TODO: last selected unit view
    std::unique_ptr<Browser> _browser;;

    //popups
    std::unique_ptr<UnitControlPopup> _unitControlPopup;
    std::unique_ptr<RouteManager> _routeManager;
    std::unique_ptr<TimelinePopup> _timelinePopup;
    std::unique_ptr<ScreenKeyboard> _keyboard;
    std::unique_ptr<FilePopup> _filePopup;
    std::unique_ptr<NewModulePopup> _newModulePopup;
    std::unique_ptr<SettingsPopup> _settingsPopup;
    std::unique_ptr<VirtualMidiKeyboard> _virtualMidiKeyboard;
    // ViewSelector * _viewSelector; //called only when need to switch from drag to target?
    //Slider mode popup * //aka AKAI MPC Live 3

    private:
    UIContext _uiContext;
    PopupManager _popManager;

    std::vector<Popup*> _popups; //only one popup must be active at the time
    
    std::unique_ptr<DragContext> _dragContext;

    MainView _currentView;
    MainView _prevView;

    BaseWidget * _gestureTarget;
    BaseWidget * _initialGestureTarget;
    BaseWidget * hitTest(BaseWidget * node, int x, int y);


    lv_obj_t * _floatingText;
    lv_timer_t * _floatingTimer;
    static void floatingTimercb(lv_timer_t * timer);

    View * getSwitchViewTarget(MainView & view);

    lv_timer_t * _playheadUpdateTimer;
    static void playheadUpdateCb(lv_timer_t * timer);

    UnitUIBase * _lastSelectedModule = nullptr;

    // std::vector<std::function<void()>> _frequentUpdateCallbacks;
};

}