// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "display/primitives/BaseWidget.h"
#include "display/utility/UIContext.h"
#include "display/popups/PopupManager.h"
#include "common/defines.h"

#include <memory>

namespace slr {
    class AudioUnitView;
    class UnitDescriptor;
    struct SequenceView;
    struct ModulationPatternView;
}

namespace UI {

class UnitUIBase;
class View;

class TopPanel;
class BottomPanel;
class GridView;
class UnitView;
class Browser;
class StepSequencerView;
class ModEngineView;

class Popup;
class UnitControlPopup;
class RouteManager;
class TimelinePopup;
class ScreenKeyboard;
class FilePopup;
class NewUnitPopup;
class SettingsPopup;
class VirtualMidiKeyboard;
class DragViewSelector;
class TargetSelectPopup;
struct ModEngineTargetManager;

class DragContext;

struct MainWindow : public BaseWidget {
    MainWindow(lv_obj_t * screen);
    ~MainWindow();
    
    void switchToView(MainView view);
    MainView currentView() const { return _currentView; }
    MainView previousView() const { return _prevView; }
    void switchToPreviousView();

    bool handleGesture(GestLib::Gesture & gesture);
    void transferGesture(BaseWidget * target, GestLib::Gestures gesture);

    void floatingText(bool warning, const std::string &text);

    void updateTimeline(const bool timeSigOrBpm);
    void updatePlayheadPosition(slr::frame_t position);

    void updateMetronomeState(bool onoff);

    void createSequenceUI(const std::shared_ptr<slr::SequenceView> view);
    void createModulationUI(const std::shared_ptr<slr::ModulationPatternView> view);
    
    void clearHittestTarget() { _initialGestureTarget = nullptr; }
    bool cancleGesture(BaseWidget * widget);

    void createUI(const slr::UnitDescriptor * desc, const std::shared_ptr<const slr::AudioUnitView> &view);
    void restoreUI(slr::ID id);
    void removeUI(slr::ID id);
    void deleteUI(slr::ID id);

    void clearUI();

    void pollUIUpdate() override;
    
    void registerFrequentUpdate(std::function<void()> clb);

    static MainWindow * inst();
    
    std::unique_ptr<TopPanel> _topPanel;
    std::unique_ptr<BottomPanel> _bottomPanel;

    //Main views
    std::unique_ptr<GridView> _gridView;
    std::unique_ptr<UnitView> _unitView; //TODO: last selected unit view
    std::unique_ptr<Browser> _browser;
    std::unique_ptr<StepSequencerView> _stepSequencerView;
    std::unique_ptr<ModEngineView> _modEngineView;

    //popups
    std::unique_ptr<UnitControlPopup> _unitControlPopup;
    std::unique_ptr<RouteManager> _routeManager;
    std::unique_ptr<TimelinePopup> _timelinePopup;
    std::unique_ptr<ScreenKeyboard> _keyboard;
    std::unique_ptr<FilePopup> _filePopup;
    std::unique_ptr<NewUnitPopup> _newUnitPopup;
    std::unique_ptr<SettingsPopup> _settingsPopup;
    std::unique_ptr<VirtualMidiKeyboard> _virtualMidiKeyboard;
    std::unique_ptr<DragViewSelector> _dragViewSelector;
    std::unique_ptr<TargetSelectPopup> _targetSelectPopup; //for step sequencer
    std::unique_ptr<ModEngineTargetManager> _modEngineTargetManagerPopup;

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

    // std::vector<std::function<void()>> _frequentUpdateCallbacks;
};

}