// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/PopupManager.h"

#include "ui/display/primitives/UnitUIBase.h"
#include "ui/display/primitives/FileView.h"
#include "ui/display/ScreenKeyboard.h"
#include "ui/display/RouteManager.h"
#include "ui/display/NewModulePopup.h"

namespace UI {

PopupManager::PopupManager() {

}

PopupManager::~PopupManager() {

}

void PopupManager::enableKeyboard(std::string initialText, std::function<void(const std::string &text)> finishCallback) {
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

void PopupManager::enableNewModulePopup() {
    _newModulePopup->update();
    _newModulePopup->activate();
    _newModulePopup->show();
}

void PopupManager::disableNewModulePopup() {
    _newModulePopup->deactivate();
    _newModulePopup->hide();
}


}