// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/primitives/View.h"
#include "ui/display/primitives/UIContext.h"

namespace UI {

View::View(BaseWidget * parent, UIContext * uicontext) 
    : BaseWidget(parent, true),
    _uictx(uicontext)
{

}

View::~View() {

}

}