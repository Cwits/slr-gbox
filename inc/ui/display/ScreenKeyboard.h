// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/display/primitives/Popup.h"

#include <functional>
#include <vector>

namespace UI {

class Button;
class UIContext;

struct ScreenKeyboard : public Popup {
    ScreenKeyboard(BaseWidget * parent, UIContext * const uictx);
    ~ScreenKeyboard();

    void update();
    void setText(std::string & text);
    void finishedCallback(std::function<void(const std::string&)> finished);

    private:
    lv_obj_t * _keyboard;
    lv_obj_t * _textArea;

    std::vector<Button*> _buttonMatrix;
    std::vector<Button*> _alternativeMatrix;
    
    Button * _backspaceBtn;
    Button * _enterBtn;
    
    Button * _dotBtn;
    Button * _slashBtn;
    Button * _clearBtn;
    
    Button * _mulButton;
    Button * _plusButton;
    Button * _spaceButton;
    // Button * _shiftBtn;
    enum class ShiftState { Disabled, Enabled, Long };
    ShiftState _shiftState;

    std::function<void(const std::string&)> _finished;


    bool handleTap(GestLib::TapGesture &tap) override;
    void initButton(Button * btn, int x, int y);
};

}