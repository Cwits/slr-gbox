// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/push/PushUIContext.h"

#include "push/PushLib.h"

#include "ui/push/MainWidget.h"

namespace PushUI {

PushUIContext::PushUIContext() {}

bool PushUIContext::tryHandleButtonDefault(PushLib::ButtonEvent &ev) {
    return _mainWidget->handleDefaultButton(ev);
}
    
bool PushUIContext::tryHandleEncoderDefault(PushLib::EncoderEvent &ev) {
    return false;
}

void PushUIContext::switchToView(PushView view) { _mainWidget->switchToView(view); }
void PushUIContext::goToPreviousView() { _mainWidget->goToPreviousView(); }
const PushView PushUIContext::currentView() const { return _mainWidget->currentView(); }
const PushView PushUIContext::previousView() const { return _mainWidget->previousView(); }


}