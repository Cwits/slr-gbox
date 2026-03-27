// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "push/PushContext.h"

#include "push/PushCore.h"
#include "push/PushPads.h"
// #include "push/PushPadLayout.h"

namespace PushLib {

void PushContext::setButtonColor(ButtonColor &clr) {
    _leds->setButtonColor(clr);
}

void PushContext::setButtonsColors(std::vector<ButtonColor> &clrs) {
    _leds->setButtonsColor(clrs);
}

void PushContext::clearButtonColors() {
    _leds->clearAllButtons();
}

void PushContext::setPadsLayout(PushPadLayout * layout) {
    _pads->setLayout(layout);
    _pads->updateRequest();
}

PushPadLayout const * PushContext::currentPadLayout() const {
    return _pads->currentLayout();
}

void PushContext::updatePadsColors() {
    _pads->updateRequest();
}

void PushContext::forceRedraw() {
    _core->redraw();
}



// void PushContext::floatingText(std::string text) {
//}


}