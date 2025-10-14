// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/display/primitives/BaseWidget.h"

namespace UI {
class UIContext;

class View : public BaseWidget {
    public:
    View(BaseWidget * parent, UIContext * const uicontext);
    virtual ~View();

    virtual void update() = 0; 

    protected:
    UIContext * const _uictx;
};

}