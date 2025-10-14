// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/ScreenKeyboard.h"

#include "ui/display/primitives/UIContext.h"
#include "ui/display/primitives/Button.h"
#include "ui/display/layoutSizes.h"
#include "ui/display/defaultColors.h"

#include "logger.h"

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


namespace UI {

ScreenKeyboard::ScreenKeyboard(BaseWidget * parent, UIContext * const uictx) :
    Popup(parent, uictx)
{
    setSize(LayoutDef::KEYBOARD_W, LayoutDef::KEYBOARD_H);
    setPos(LayoutDef::KEYBOARD_X, LayoutDef::KEYBOARD_Y);
    lv_obj_set_style_bg_color(lvhost(), KEYBOARD_BACKGROUND_COLOR, 0);
    // _keyboard = lv_keyboard_create(lvhost());
    // lv_obj_set_height(_keyboard, LVGL_KEYBOARD_H);

    _textArea = lv_textarea_create(lvhost());
    lv_obj_align(_textArea, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_set_size(_textArea, LayoutDef::KB_TEXT_AREA_W, LayoutDef::KB_TEXT_AREA_H);
    lv_textarea_set_one_line(_textArea, true);
    lv_obj_set_style_text_font(_textArea, &DEFAULT_FONT, 0);
    // lv_obj_add_event_cb(_textArea, ta_event_cb, LV_EVENT_ALL, _keyboard);
    // // lv_obj_set_style_bg_color(_textArea, lv_color_hex(0xffffff), 0);

    int rowx = 5;
    int rowy = LayoutDef::KB_TEXT_AREA_H + LayoutDef::DEFAULT_MARGIN;

    for(int i=0; i<LayoutDef::KEYS_ROW_NUM; ++i) {
        Button * btn = new Button(this, num_map[i]);
        initButton(btn, rowx, rowy);
        rowx += (LayoutDef::KB_BUTTON_SIZE + LayoutDef::KB_BUTTON_MARGIN);
    }

    rowx = 5;
    rowy += LayoutDef::KB_BUTTON_SIZE + LayoutDef::KB_BUTTON_MARGIN;
    for(int i=0; i<LayoutDef::KEYS_ROW_1; ++i) {
        Button * btn = new Button(this, kb_map_upper[i]);
        initButton(btn, rowx, rowy);
        rowx += (LayoutDef::KB_BUTTON_SIZE + LayoutDef::KB_BUTTON_MARGIN);
    }

    rowx = 5;
    rowy += LayoutDef::KB_BUTTON_SIZE + LayoutDef::KB_BUTTON_MARGIN;
    for(int i=0; i<LayoutDef::KEYS_ROW_2; ++i) {
        Button * btn = new Button(this, kb_map_upper[i+LayoutDef::KEYS_ROW_1]);
        initButton(btn, rowx, rowy);
        rowx += (LayoutDef::KB_BUTTON_SIZE + LayoutDef::KB_BUTTON_MARGIN);
    }

    rowx = 5;
    rowy += LayoutDef::KB_BUTTON_SIZE + LayoutDef::KB_BUTTON_MARGIN;
    for(int i=0; i<LayoutDef::KEYS_ROW_3; ++i) {
        Button * btn = new Button(this, kb_map_upper[i+LayoutDef::KEYS_ROW_1+LayoutDef::KEYS_ROW_2]);
        initButton(btn, rowx, rowy);
        rowx += (LayoutDef::KB_BUTTON_SIZE + LayoutDef::KB_BUTTON_MARGIN);
    }

    rowx = LayoutDef::KEYBOARD_W - (LayoutDef::KB_BUTTON_SIZE + LayoutDef::KB_BUTTON_MARGIN);
    rowy = LayoutDef::KEYBOARD_H - (LayoutDef::KB_BUTTON_SIZE + LayoutDef::KB_BUTTON_MARGIN);
    _slashBtn = new Button(this, "/");    
    _slashBtn->setSize(LayoutDef::KB_BUTTON_SIZE, LayoutDef::KB_BUTTON_SIZE);
    _slashBtn->setPos(rowx, rowy);
    _slashBtn->setFont(&DEFAULT_FONT);
    _slashBtn->setCallback([this]() {
        LOG_INFO("Pressed Slash");
        lv_textarea_add_char(this->_textArea, '/');
    });

    rowx -= (LayoutDef::KB_BUTTON_SIZE + LayoutDef::KB_BUTTON_MARGIN);
    _dotBtn = new Button(this, ".");
    _dotBtn->setSize(LayoutDef::KB_BUTTON_SIZE, LayoutDef::KB_BUTTON_SIZE);
    _dotBtn->setPos(rowx, rowy);
    _dotBtn->setFont(&DEFAULT_FONT);
    _dotBtn->setCallback([this]() {
        LOG_INFO("Pressed Dot");
        lv_textarea_add_char(this->_textArea, '.');
    });

    _backspaceBtn = new Button(this, LV_SYMBOL_BACKSPACE);
    _backspaceBtn->setSize(LayoutDef::KB_BACKSPACE_W, LayoutDef::KB_BACKSPACE_H);
    _backspaceBtn->setPos(LayoutDef::KB_BACKSPACE_X, LayoutDef::KB_BACKSPACE_Y);
    _backspaceBtn->setFont(&DEFAULT_FONT);
    _backspaceBtn->setCallback([this]() {
        LOG_INFO("Pressed Backspace");
        lv_textarea_delete_char(this->_textArea);
    });

    _enterBtn = new Button(this, LV_SYMBOL_OK);
    _enterBtn->setSize(LayoutDef::KB_ENTER_W, LayoutDef::KB_ENTER_H);
    _enterBtn->setPos(LayoutDef::KB_ENTER_X, LayoutDef::KB_ENTER_Y);
    _enterBtn->setFont(&DEFAULT_FONT);
    _enterBtn->setCallback([this]() {
        LOG_INFO("Pressed Enter");
        const std::string text = std::string(lv_textarea_get_text(this->_textArea));
        // LOG_INFO("Text from keyboard: %s", text.c_str());
        if(this->_finished) {
            this->_finished(text);
        }

        this->_uictx->_popManager->disableKeyboard();
    });

    // rowx -= (KB_BUTTON_SIZE + KB_BUTTON_MARGIN);
    rowx -= (LayoutDef::KB_BUTTON_SIZE + LayoutDef::KB_BUTTON_MARGIN);
    _mulButton = new Button(this, "Mul");
    _mulButton->setSize(LayoutDef::KB_BUTTON_SIZE, LayoutDef::KB_BUTTON_SIZE);
    _mulButton->setPos(rowx, rowy);
    _mulButton->setFont(&DEFAULT_FONT);
    _mulButton->setCallback([this]() {
        LOG_INFO("Pressed *");
        lv_textarea_add_char(this->_textArea, '*');
    });
    
    rowx -= (LayoutDef::KB_BUTTON_SIZE + LayoutDef::KB_BUTTON_MARGIN);
    _plusButton = new Button(this, "Plus");
    _plusButton->setSize(LayoutDef::KB_BUTTON_SIZE, LayoutDef::KB_BUTTON_SIZE);
    _plusButton->setPos(rowx, rowy);
    _plusButton->setFont(&DEFAULT_FONT);
    _plusButton->setCallback([this]() {
        LOG_INFO("Pressed +");
        lv_textarea_add_char(this->_textArea, '+');
    });

    rowx -= (LayoutDef::KB_SPACE_W + LayoutDef::KB_BUTTON_MARGIN);
    _spaceButton = new Button(this, "Space");
    _spaceButton->setSize(LayoutDef::KB_SPACE_W, LayoutDef::KB_SPACE_H);
    _spaceButton->setPos(rowx, rowy);
    _spaceButton->setFont(&DEFAULT_FONT);
    _spaceButton->setCallback([this]() {
        LOG_INFO("Pressed Space");
        lv_textarea_add_char(this->_textArea, ' ');
    });
    //shift


    rowx = width() - (LayoutDef::KB_CLEAR_W + LayoutDef::KB_BUTTON_MARGIN);
    rowy = height() - (LayoutDef::KB_BUTTON_SIZE + LayoutDef::KB_BUTTON_MARGIN + 
                        LayoutDef::KB_BUTTON_SIZE + LayoutDef::KB_BUTTON_MARGIN);
    _clearBtn = new Button(this, "Clear");
    _clearBtn->setSize(LayoutDef::KB_CLEAR_W, LayoutDef::KB_CLEAR_H);
    _clearBtn->setPos(rowx, rowy);
    _clearBtn->setFont(&DEFAULT_FONT);
    _clearBtn->setCallback([this]() {
        LOG_INFO("Pressed Clear");
        lv_textarea_set_text(this->_textArea, "");
    });
}

ScreenKeyboard::~ScreenKeyboard() {
    // lv_obj_delete(_keyboard);
    lv_obj_delete(_textArea);
    for(Button *b : _buttonMatrix) {
        delete b;
    }
    for(Button *b : _alternativeMatrix) {
        delete b;
    }
    delete _backspaceBtn;
    delete _enterBtn;
    delete _dotBtn;
    delete _slashBtn;
    delete _clearBtn;
    
    delete _mulButton;
    delete _plusButton;
    delete _spaceButton;
}

void ScreenKeyboard::setText(std::string & text) {
    lv_textarea_set_text(_textArea, text.c_str());
}

void ScreenKeyboard::finishedCallback(std::function<void(const std::string&)> finished) {
    _finished = finished;
}

bool ScreenKeyboard::handleTap(GestLib::TapGesture &tap) {
    int notAbsX = tap.x - getX();
    int notAbsY = tap.y - getY();

    // lv_obj_add_event_cb(_keyboard, increment_on_click, LV_EVENT_CLICKED, &num1);
    // lv_keyboard_def_event_cb(LV_EVENT_CLICKED);
    // lv_obj_send_event(_keyboard, LV_EVENT_CLICKED, 0);
    return true;
}


void ScreenKeyboard::initButton(Button * btn, int x, int y) {
    btn->setSize(LayoutDef::KB_BUTTON_SIZE, LayoutDef::KB_BUTTON_SIZE);
    btn->setPos(x, y);
    btn->setFont(&DEFAULT_FONT);
    btn->setCallback([this, btn]() {
        LOG_INFO("Pressed %s", btn->text().c_str());
        lv_textarea_add_char(this->_textArea, btn->text()[0]);
    });
}


}