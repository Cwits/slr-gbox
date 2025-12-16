// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <string>
#include "defines.h"

namespace UI {

class BaseWidget;
class UnitUIBase;
class UnitControlPopup;
class RouteManager;
class TimelinePopup;
class ScreenKeyboard;
class FilePopup;
class FileView;
class NewModulePopup;
class SettingsPopup;
class VirtualMidiKeyboard;

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

    void enableFilePopup(FileView * file);
    void setFilePopupPosition(int x, int y);
    void disableFilePopup();

    void enableNewModulePopup();
    void disableNewModulePopup();

    void enableSettingsPopup();
    void disableSettingsPopup();

    void enableMidiKeyboard();
    void disableMidiKeyboard();

    private:
    UnitControlPopup * _unitControlPopup;
    ScreenKeyboard * _screenKeyboard;
    RouteManager * _routeManager;
    FilePopup * _filePopup;
    NewModulePopup * _newModulePopup;
    SettingsPopup * _settingsPopup;
    VirtualMidiKeyboard * _virtualMidiKeyboard;

    friend class MainWindow;
};

}