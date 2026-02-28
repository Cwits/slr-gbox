// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "push/PushContext.h"
#include "push/PushPads.h"
#include "push/PushPadLayout.h"

namespace PushLib {

void PushContext::setPadsNotes() {

}

void PushContext::setPadsColors(unsigned char root, unsigned char inScale, unsigned char offScale, unsigned char pressed) {
    _pads->_layout.setColors(root, inScale, offScale, pressed);
    _pads->updateRequest();
}

void PushContext::setPadsChromatic(bool chromatic) {
    _pads->_layout.setChromatic(chromatic);
    _pads->updateRequest();
}

const bool PushContext::isPadsChromatic() {
    return _pads->_layout.chromatic();
}

void PushContext::setPadsScale(const Scales scale) {
    _pads->_layout.setScale(scale);
    _pads->updateRequest();
}

void PushContext::setPadsLayout(const LayoutStyle layout) {
    _pads->_layout.setLayout(layout);
    _pads->updateRequest();
}

void PushContext::setPadsCustom() {

}



// void PushContext::floatingText(std::string text) {
//}


}