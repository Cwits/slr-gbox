// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "common/defines.h"

#include <functional>
#include <string>

namespace UI {

class BaseWidget;
class UnitUIBase;
class UnitControlPopup;
class RouteManager;
class TimelinePopup;
class ScreenKeyboard;
class ClipPreviewPopup;
class ClipPreview;
class NewUnitPopup;
class SettingsPopup;
class VirtualMidiKeyboard;
class DragViewSelector;
struct TargetSelectPopup;
struct ModEngineTargetManager;

struct PopupManager {
    PopupManager();
    ~PopupManager();

    TimelinePopup * _timelinePopup;

    void enableKeyboard(std::string initialText, 
                    std::function<void(const std::string &text)> finishCallback);
    void disableKeyboard();

    void enableUnitControl(UnitUIBase * unit, BaseWidget * parent);
    void disableUnitControl();

    void enableRouteManager(slr::ID unitId);
    void disableRouteManager();

    void enableClipPreviewPopup(ClipPreview * file);
    void setClipPreviewPopupPosition(int x, int y);
    void disableClipPreviewPopup();

    void enableNewUnitPopup();
    void disableNewUnitPopup();

    void enableSettingsPopup();
    void disableSettingsPopup();

    void enableMidiKeyboard();
    void disableMidiKeyboard();

    void enableDragSelector();
    void disableDragSelector();

    void enableTargetSelectPopup();
    void disableTargetSelectPopup();

    void enableModEngineTargetManager();
    void disableModEngineTargetManager();
    
    private:
    UnitControlPopup * _unitControlPopup;
    ScreenKeyboard * _screenKeyboard;
    RouteManager * _routeManager;
    ClipPreviewPopup * _clipPreviewPopup;
    NewUnitPopup * _newUnitPopup;
    SettingsPopup * _settingsPopup;
    VirtualMidiKeyboard * _virtualMidiKeyboard;
    DragViewSelector * _dragViewSelector;
    TargetSelectPopup * _targetSelectPopup;
    ModEngineTargetManager * _modEngineTargetManagerPopup;

    friend class RootWindow;
};

}