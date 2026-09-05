// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "push/utility/PushUIContext.h"

#include "push/core/PushLib.h"
#include "push/core/PushContext.h"

#include "push/elements/RootWidget.h"
#include "push/elements/GridWidget.h"
#include "push/elements/UnitWidget.h"
#include "push/primitives/UnitUIBase.h"
// #include "ui/push/primitives/ModuleUIBase.h"

namespace PushUI {

PushUIContext::PushUIContext() {}
PushUIContext::~PushUIContext() {}

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

PushLib::Widget * PushUIContext::gridWidget() const { return _gridWidget; }
PushLib::Widget * PushUIContext::unitWidget() const { return _unitWidget; }

void PushUIContext::forceRedraw() {
    _pctx->forceRedraw();
}

}