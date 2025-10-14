// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/display/primitives/Popup.h"

namespace UI {

class UIContext;
class Button;

struct NewModulePopup : public Popup {
    NewModulePopup(BaseWidget *parent, UIContext * const uictx);
    ~NewModulePopup();

    void update();

    private:
    
    Button * _btnTrack;
    Button * _btnMixer;
};

}