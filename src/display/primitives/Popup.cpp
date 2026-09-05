// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display/primitives/Popup.h"
#include "display/utility/UIContext.h"

namespace UI {

Popup::Popup(BaseWidget * parent, UIContext * const uictx) :
    BaseWidget(parent, true),
    _uictx(uictx)
{
    _active = false;
}

Popup::~Popup() {

}

void Popup::deactivate() { 
    forcedClose(); 
    _active = false;
    
}

}