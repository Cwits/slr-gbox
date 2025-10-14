// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/display/primitives/BaseWidget.h"

namespace UI {

class UIContext;

struct Popup : public BaseWidget {
    Popup(BaseWidget * parent, UIContext * const uictx);
    virtual ~Popup();

    bool active() const { return _active; }
    void activate() { _active = true; }
    void deactivate();
    
    protected:
    UIContext * const _uictx;
    
    private:
    bool _active;

    //use this for clearing when popup closed by tapping outside its region
    virtual void forcedClose() {}
};

}