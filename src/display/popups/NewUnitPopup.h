// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "display/primitives/Popup.h"
#include <memory>

namespace UI {

class UIContext;
class Button;

struct NewUnitPopup : public Popup {
    NewUnitPopup(BaseWidget *parent, UIContext * const uictx);
    ~NewUnitPopup();

    void update();

    private:
    
    std::unique_ptr<Button> _btnTrack;
    std::unique_ptr<Button> _btnMixer;
    std::unique_ptr<Button> _btnOsc;
    std::unique_ptr<Button> _btnSampler;
};

}