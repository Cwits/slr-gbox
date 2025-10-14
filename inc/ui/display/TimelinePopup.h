// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/display/primitives/Popup.h"

namespace UI {

class Button;
class Label;
class UIContext;
//opens from bottom panel bpm and time signature labels
struct TimelinePopup : public Popup {
    TimelinePopup(BaseWidget * parent, UIContext * const uictx);
    ~TimelinePopup();

    void update();
    private:
    Label * _bpm;
    Label * _timeSignature;

    Button * _applyBtn;
};

}