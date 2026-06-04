// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/PopupManager.h"

#include "ui/display/primitives/UnitUIBase.h"
#include "ui/display/primitives/FileView.h"
#include "ui/display/ScreenKeyboard.h"
#include "ui/display/RouteManager.h"
#include "ui/display/NewUnitPopup.h"
#include "ui/display/SettingsPopup.h"
#include "ui/display/VirtualMidiKeyboard.h"
#include "ui/display/DragViewSelector.h"

namespace UI {

PopupManager::PopupManager() {

}

PopupManager::~PopupManager() {

}

void PopupManager::enableKeyboard(std::string initialText, std::function<void(const std::string &text)> finishCallback) {
    lv_obj_move_to_index(_screenKeyboard->lvhost(), -1);
    _screenKeyboard->setText(initialText);
    _screenKeyboard->finishedCallback(finishCallback);
    _screenKeyboard->activate();
    _screenKeyboard->show();
}

void PopupManager::disableKeyboard() {
    _screenKeyboard->deactivate();
    _screenKeyboard->hide();
}

void PopupManager::enableUnitControl(UnitUIBase * unit, BaseWidget * parent) {
    
    _unitControlPopup->_currentUnit = unit;
    int cx = _unitControlPopup->getX();
    _unitControlPopup->setPos(cx, parent->getY());   
    _unitControlPopup->activate();
    _unitControlPopup->show();
}

void PopupManager::disableUnitControl() {
    _unitControlPopup->deactivate();
    _unitControlPopup->hide();
}

void PopupManager::enableRouteManager(slr::ID unitId) {
    _routeManager->_currentUnitId = unitId;
    _routeManager->update();
    _routeManager->activate();
    _routeManager->show();
}

void PopupManager::disableRouteManager() {
    _routeManager->deactivate();
    _routeManager->hide();
}

void PopupManager::enableFilePopup(FileView * file) {
    _filePopup->setItem(file);
    _filePopup->update();
    _filePopup->activate();
    _filePopup->show();
}

void PopupManager::setFilePopupPosition(int x, int y) {
    _filePopup->setPos(x, y);
}

void PopupManager::disableFilePopup() {
    _filePopup->deactivate();
    _filePopup->hide();
}

void PopupManager::enableNewUnitPopup() {
    _newUnitPopup->update();
    _newUnitPopup->activate();
    _newUnitPopup->show();
}

void PopupManager::disableNewUnitPopup() {
    _newUnitPopup->deactivate();
    _newUnitPopup->hide();
}

void PopupManager::enableSettingsPopup() {
    _settingsPopup->update();
    _settingsPopup->activate();
    _settingsPopup->show();
}

void PopupManager::disableSettingsPopup() {
    _settingsPopup->deactivate();
    _settingsPopup->hide();
}

void PopupManager::enableMidiKeyboard() {
    // _virtualMidiKeyboard->update();
    _virtualMidiKeyboard->activate();
    _virtualMidiKeyboard->show();
}

void PopupManager::disableMidiKeyboard() {
    _virtualMidiKeyboard->deactivate();
    _virtualMidiKeyboard->hide();
}

void PopupManager::enableDragSelector() {
    _dragViewSelector->activate();
    _dragViewSelector->show();
}

void PopupManager::disableDragSelector() {
    _dragViewSelector->deactivate();
    _dragViewSelector->hide();
}

    


}