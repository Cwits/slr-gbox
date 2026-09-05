// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display/popups/PopupManager.h"

#include "display/primitives/UnitUIBase.h"
#include "display/primitives/FileView.h"
#include "display/popups/ScreenKeyboard.h"
#include "display/popups/RouteManager.h"
#include "display/popups/NewUnitPopup.h"
#include "display/popups/SettingsPopup.h"
#include "display/popups/VirtualMidiKeyboard.h"
#include "display/popups/DragViewSelector.h"
#include "display/popups/ModEngineTargetManager.h"

#include "display/elements/StepSequencerView.h"

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

void PopupManager::enableTargetSelectPopup() {
    _targetSelectPopup->update();
    _targetSelectPopup->activate();
    _targetSelectPopup->show();
}

void PopupManager::disableTargetSelectPopup() {
    _targetSelectPopup->deactivate();
    _targetSelectPopup->hide();
}


void PopupManager::enableModEngineTargetManager() {
    _modEngineTargetManagerPopup->update();
    _modEngineTargetManagerPopup->activate();
    _modEngineTargetManagerPopup->show();
}

void PopupManager::disableModEngineTargetManager() {
    _modEngineTargetManagerPopup->deactivate();
    _modEngineTargetManagerPopup->hide();
}
    


}