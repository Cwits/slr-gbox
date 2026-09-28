// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display/popups/ScreenKeyboard.h"

#include "display/primitives/Button.h"
#include "display/utility/UIContext.h"
#include "display/utility/layoutSizes.h"
#include "display/utility/defaultColors.h"

#include "common/logger.h"

static const char * num_map[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "0" };
static const char * kb_map_upper[] = {
    "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
    "A", "S", "D", "F", "G", "H", "J", "K", "L",
    "Z", "X", "C", "V", "B", "N", "M"
};

static const char * kb_map_lower[] = {
    "q", "w", "e", "r", "t", "y", "u", "i", "o", "p",
    "a", "s", "d", "f", "g", "h", "j", "k", "l",
    "z", "x", "c", "v", "b", "n", "m"
};

constexpr int KEY_ROW_START_X = 500;

namespace UI {

ScreenKeyboard::ScreenKeyboard(BaseWidget * parent, UIContext * const uictx) :
    Popup(parent, uictx)
{
    setSize(Layout::KEYBOARD_W, Layout::KEYBOARD_H);
    setPos(Layout::KEYBOARD_X, Layout::KEYBOARD_Y);
    lv_obj_set_style_bg_color(lvhost(), KEYBOARD_BACKGROUND_COLOR, 0);
    // _keyboard = lv_keyboard_create(lvhost());
    // lv_obj_set_height(_keyboard, LVGL_KEYBOARD_H);

    _textArea = lv_textarea_create(lvhost());
    lv_obj_align(_textArea, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_set_size(_textArea, Layout::KB_TEXT_AREA_W, Layout::KB_TEXT_AREA_H);
    lv_textarea_set_one_line(_textArea, true);
    lv_obj_set_style_text_font(_textArea, &DEFAULT_FONT, 0);
    // lv_obj_add_event_cb(_textArea, ta_event_cb, LV_EVENT_ALL, _keyboard);
    // // lv_obj_set_style_bg_color(_textArea, lv_color_hex(0xffffff), 0);

    // ------------- First Row ---------------- //
    // nums and backspace
    int rowx = KEY_ROW_START_X;
    int rowy = Layout::KB_TEXT_AREA_H + Layout::Margin;
    for(int i=0; i<Layout::KEYS_ROW_NUM; ++i) {
        std::unique_ptr<Button> btn = std::make_unique<Button>(this, num_map[i]);
        initButton(btn.get(), rowx, rowy);
        rowx += (Layout::KB_BUTTON_SIZE + Layout::KB_BUTTON_MARGIN);
        _numMatrix.push_back(std::move(btn));
    }

    _btnBackspace = std::make_unique<Button>(this, LV_SYMBOL_BACKSPACE);
    _btnBackspace->setSize(Layout::KB_BACKSPACE_W, Layout::KB_BACKSPACE_H);
    _btnBackspace->setPos(Layout::KB_BACKSPACE_X, Layout::KB_BACKSPACE_Y);
    _btnBackspace->setFont(&DEFAULT_FONT);
    _btnBackspace->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        LOG_INFO("Pressed Backspace");
        lv_textarea_delete_char(this->_textArea);
        return true;
    });

    // ---------------- Second Row -------------
    //qwert... and Clear

    rowx = KEY_ROW_START_X;
    rowy += Layout::KB_BUTTON_SIZE + Layout::KB_BUTTON_MARGIN;
    for(int i=0; i<Layout::KEYS_ROW_1; ++i) {
        std::unique_ptr<Button> btn = std::make_unique<Button>(this, kb_map_upper[i]);
        initButton(btn.get(), rowx, rowy);
        rowx += (Layout::KB_BUTTON_SIZE + Layout::KB_BUTTON_MARGIN);
        _buttonMatrix.push_back(std::move(btn));
    }

    rowx = width() - (Layout::KB_CLEAR_W + Layout::KB_BUTTON_MARGIN);
    _btnClear = std::make_unique<Button>(this, "Clear");
    _btnClear->setSize(Layout::KB_CLEAR_W, Layout::KB_CLEAR_H);
    _btnClear->setPos(rowx, rowy);
    _btnClear->setFont(&DEFAULT_FONT);
    _btnClear->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        LOG_INFO("Pressed Clear");
        lv_textarea_set_text(this->_textArea, "");
        return true;
    });

    // ---------------- Third Row ---------------
    //  asdf..., Plus, Mul

    rowx = (KEY_ROW_START_X+Layout::KB_BUTTON_SIZE/2);
    rowy += Layout::KB_BUTTON_SIZE + Layout::KB_BUTTON_MARGIN;
    for(int i=0; i<Layout::KEYS_ROW_2; ++i) {
        std::unique_ptr<Button> btn = std::make_unique<Button>(this, kb_map_upper[i+Layout::KEYS_ROW_1]);
        initButton(btn.get(), rowx, rowy);
        rowx += (Layout::KB_BUTTON_SIZE + Layout::KB_BUTTON_MARGIN);
        _buttonMatrix.push_back(std::move(btn));
    }

    rowx += (Layout::KB_BUTTON_SIZE*2);
    _btnPlus = std::make_unique<Button>(this, "Plus");
    _btnPlus->setSize(Layout::KB_BUTTON_SIZE, Layout::KB_BUTTON_SIZE);
    _btnPlus->setPos(rowx, rowy);
    _btnPlus->setFont(&DEFAULT_FONT);
    _btnPlus->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        LOG_INFO("Pressed +");
        lv_textarea_add_char(this->_textArea, '+');
        return true;
    });

    rowx += (Layout::KB_BUTTON_SIZE + Layout::KB_BUTTON_MARGIN);
    _btnMul = std::make_unique<Button>(this, "Mul");
    _btnMul->setSize(Layout::KB_BUTTON_SIZE, Layout::KB_BUTTON_SIZE);
    _btnMul->setPos(rowx, rowy);
    _btnMul->setFont(&DEFAULT_FONT);
    _btnMul->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        LOG_INFO("Pressed *");
        lv_textarea_add_char(this->_textArea, '*');
        return true;
    });

    // ---------------- Forth Row --------------
    // Shift, Space, zxcvb..., dot('.'), slash('/') and Enter

    rowy += Layout::KB_BUTTON_SIZE + Layout::KB_BUTTON_MARGIN;
    rowx = (Layout::KB_BUTTON_MARGIN);
    _btnShift = std::make_unique<Button>(this, "Shift");
    _btnShift->setSize(Layout::KB_SHIFT_W, Layout::KB_SHIFT_H);
    _btnShift->setPos(rowx, rowy);
    _btnShift->setFont(&DEFAULT_FONT);
    _btnShift->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        // LOG_INFO("Pressed Space");
        // lv_textarea_add_char(this->_textArea, ' ');
        if(this->_shiftState == ScreenKeyboard::ShiftState::Enabled) {
            this->toCase(false);
            this->_shiftState = ScreenKeyboard::ShiftState::Disabled;
        } else {
            this->toCase(true);
            this->_shiftState = ScreenKeyboard::ShiftState::Enabled;
        }
        return true;
    });
    _shiftState = ScreenKeyboard::ShiftState::Enabled;
    
    rowx = (Layout::KB_SPACE_W + (Layout::KB_BUTTON_MARGIN*2));
    _btnSpace = std::make_unique<Button>(this, "Space");
    _btnSpace->setSize(Layout::KB_SPACE_W, Layout::KB_SPACE_H);
    _btnSpace->setPos(rowx, rowy);
    _btnSpace->setFont(&DEFAULT_FONT);
    _btnSpace->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        LOG_INFO("Pressed Space");
        lv_textarea_add_char(this->_textArea, ' ');
        return true;
    });

    rowx = KEY_ROW_START_X+Layout::KB_BUTTON_SIZE+Layout::KB_BUTTON_MARGIN+(Layout::KB_BUTTON_SIZE/2);
    for(int i=0; i<Layout::KEYS_ROW_3; ++i) {
        std::unique_ptr<Button> btn = std::make_unique<Button>(this, kb_map_upper[i+Layout::KEYS_ROW_1+Layout::KEYS_ROW_2]);
        initButton(btn.get(), rowx, rowy);
        rowx += (Layout::KB_BUTTON_SIZE + Layout::KB_BUTTON_MARGIN);
        _buttonMatrix.push_back(std::move(btn));
    }

    rowx += (Layout::KB_BUTTON_SIZE*3 + Layout::KB_BUTTON_MARGIN);
    _btnDot = std::make_unique<Button>(this, ".");
    _btnDot->setSize(Layout::KB_BUTTON_SIZE, Layout::KB_BUTTON_SIZE);
    _btnDot->setPos(rowx, rowy);
    _btnDot->setFont(&DEFAULT_FONT);  
    _btnDot->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        LOG_INFO("Pressed Dot");
        lv_textarea_add_char(this->_textArea, '.');
        return true;
    });

    rowx += (Layout::KB_BUTTON_SIZE + Layout::KB_BUTTON_MARGIN);
    _btnSlash = std::make_unique<Button>(this, "/");    
    _btnSlash->setSize(Layout::KB_BUTTON_SIZE, Layout::KB_BUTTON_SIZE);
    _btnSlash->setPos(rowx, rowy);
    _btnSlash->setFont(&DEFAULT_FONT);
    _btnSlash->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        LOG_INFO("Pressed Slash");
        lv_textarea_add_char(this->_textArea, '/');
        return true;
    });

    _btnEnter = std::make_unique<Button>(this, LV_SYMBOL_OK);
    _btnEnter->setSize(Layout::KB_ENTER_W, Layout::KB_ENTER_H);
    _btnEnter->setPos(Layout::KB_ENTER_X, Layout::KB_ENTER_Y);
    _btnEnter->setFont(&DEFAULT_FONT);
    _btnEnter->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        LOG_INFO("Pressed Enter");
        const std::string text = std::string(lv_textarea_get_text(this->_textArea));
        // LOG_INFO("Text from keyboard: %s", text.c_str());
        if(this->_finished) {
            this->_finished(text);
        }

        this->_uictx->_popManager->disableKeyboard();
        return true;
    });
}

ScreenKeyboard::~ScreenKeyboard() {
    // lv_obj_delete(_keyboard);
    lv_obj_delete(_textArea);
    _numMatrix.clear();
    _buttonMatrix.clear();
    // for(Button *b : _numMatrix) {
    //     delete b;
    // }
    // for(Button *b : _buttonMatrix) {
    //     delete b;
    // }
    // delete _backspaceBtn;
    // delete _enterBtn;
    // delete _dotBtn;
    // delete _slashBtn;
    // delete _clearBtn;
    
    // delete _mulButton;
    // delete _plusButton;
    // delete _spaceButton;
    // delete _shiftButton;
}

void ScreenKeyboard::setText(std::string & text) {
    lv_textarea_set_text(_textArea, text.c_str());
}

void ScreenKeyboard::finishedCallback(std::function<void(const std::string&)> finished) {
    _finished = finished;
}

void ScreenKeyboard::toCase(bool upper) {
    const char ** ptr = nullptr;
    if(upper) {
        //to upper case
        ptr = kb_map_upper;
    } else {
        //to lower case
        ptr = kb_map_lower;
    }

    for(std::size_t i=0; i<_buttonMatrix.size(); ++i) {
        Button * btn = _buttonMatrix.at(i).get();
        btn->setText(ptr[i]);
    }
}

// bool ScreenKeyboard::handleTap(GestLib::TapGesture &tap) {
//     int notAbsX = tap.x - getX();
//     int notAbsY = tap.y - getY();

//     // lv_obj_add_event_cb(_keyboard, increment_on_click, LV_EVENT_CLICKED, &num1);
//     // lv_keyboard_def_event_cb(LV_EVENT_CLICKED);
//     // lv_obj_send_event(_keyboard, LV_EVENT_CLICKED, 0);
//     return true;
// }


void ScreenKeyboard::initButton(Button * btn, int x, int y) {
    btn->setSize(Layout::KB_BUTTON_SIZE, Layout::KB_BUTTON_SIZE);
    btn->setPos(x, y);
    btn->setFont(&DEFAULT_FONT);
    btn->tapCallback([this, btn](const GestLib::TapGesture &tap) -> bool {
        LOG_INFO("Pressed %s", btn->text().c_str());
        lv_textarea_add_char(this->_textArea, btn->text()[0]);
        if(this->_shiftState == ScreenKeyboard::ShiftState::Enabled) {
            this->toCase(false);
            this->_shiftState = ScreenKeyboard::ShiftState::Disabled;
        }
        return true;
    });
}


}