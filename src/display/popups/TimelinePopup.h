// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "display/primitives/Popup.h"
#include <memory>

namespace UI {

class Button;
class Label;
class UIContext;
struct Timeline;
struct Checkbox;

//opens from bottom panel bpm and time signature labels
struct TimelinePopup : public Popup {
    TimelinePopup(BaseWidget * parent, UIContext * const uictx);
    ~TimelinePopup();

    void update();

    Timeline * _current = nullptr;

    private:
    std::unique_ptr<Label> _lblGridSize;
    std::unique_ptr<Checkbox> _chbTriplets;
    

    std::unique_ptr<Button> _applyBtn;
};

}