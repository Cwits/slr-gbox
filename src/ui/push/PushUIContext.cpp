// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/push/PushUIContext.h"

#include "push/PushLib.h"

#include "ui/push/RootWidget.h"
#include "ui/push/primitives/ModuleUIBase.h"

namespace PushUI {

PushUIContext::PushUIContext() {}

bool PushUIContext::tryHandleButtonDefault(PushLib::ButtonEvent &ev) {
    return _rootWidget->handleDefaultButton(ev);
}
    
bool PushUIContext::tryHandleEncoderDefault(PushLib::EncoderEvent &ev) {
    return false;
}

void PushUIContext::switchToView(PushView view) { _rootWidget->switchToView(view); }
void PushUIContext::goToPreviousView() { _rootWidget->goToPreviousView(); }
const PushView PushUIContext::currentView() const { return _rootWidget->currentView(); }
const PushView PushUIContext::previousView() const { return _rootWidget->previousView(); }


}