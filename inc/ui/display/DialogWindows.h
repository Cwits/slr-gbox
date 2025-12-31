// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "ui/display/primitives/Popup.h"

#include <string>

namespace UI {


class UIContext;
class Label;
class Button;

struct InfoDialog : public Popup {
    InfoDialog(BaseWidget * parent, UIContext * const uictx);
    ~InfoDialog();

    private:
    std::string _text;
    Button * _btnOk;
};

struct DecisionDialog : public Popup {
    DecisionDialog(BaseWidget * parent, UIContext * const uictx);
    ~DecisionDialog();

    private:
    std::string _text;
    Button * _btnOk;
    Button * _btnCancle;
};

}