// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "display/primitives/Popup.h"

#include <functional>
#include <vector>
#include <memory>

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

    std::vector<std::unique_ptr<Button>> _numMatrix;
    std::vector<std::unique_ptr<Button>> _buttonMatrix;
    // std::vector<Button*> _alternativeMatrix;
    
    std::unique_ptr<Button> _btnBackspace;
    std::unique_ptr<Button> _btnEnter;
    
    std::unique_ptr<Button> _btnDot;
    std::unique_ptr<Button> _btnSlash;
    std::unique_ptr<Button> _btnClear;
    
    std::unique_ptr<Button> _btnMul;
    std::unique_ptr<Button> _btnPlus;
    std::unique_ptr<Button> _btnSpace;
    std::unique_ptr<Button> _btnShift;
    // Button * _shiftBtn;
    enum class ShiftState { Disabled, Enabled };
    ShiftState _shiftState;

    std::function<void(const std::string&)> _finished;

    void toCase(bool upper);

    // bool handleTap(GestLib::TapGesture &tap);
    void initButton(Button * btn, int x, int y);
};

}