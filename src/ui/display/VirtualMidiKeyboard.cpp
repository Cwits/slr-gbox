// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/VirtualMidiKeyboard.h"
#include "ui/display/layoutSizes.h"
#include "ui/display/defaultColors.h"

#include "core/Events.h"

#include "logger.h"

constexpr int WHITE_KEYS_COUNT = 14;
constexpr int BLACK_KEYS_COUNT = 10;
constexpr int WHITE_WIDTH = UI::LayoutDef::TOTAL_WIDTH/WHITE_KEYS_COUNT; //two octaves
constexpr int BLACK_WIDTH = WHITE_WIDTH - (WHITE_WIDTH/3);
constexpr int _defaultWhiteNotes[WHITE_KEYS_COUNT] = { 
    36, 38, 40, 41, 43, 45, 47, 
    48, 50, 52, 53, 55, 57, 59 
};
constexpr int _defaultBlackNotes[BLACK_KEYS_COUNT] = { 
    37, 39, 42, 44, 46, 
    49, 51, 54, 56, 58 
};
const std::string _whiteLetters[7] = {
    "C", "D", "E", "F", "G", "A", "B"
};

lv_style_t whiteStyle;
lv_style_t blackStyle;

namespace UI {

VirtualMidiKeyboard::VirtualMidiKeyboard(BaseWidget * parent, UIContext * const uictx) :
    Popup(parent, uictx)    
{
    const int height = parent->height()/2;
    setSize(parent->width(), height);
    setPos(0, height);
    setColor(lv_color_hex(0x03b112));

    lv_style_init(&whiteStyle);
    lv_style_set_margin_all(&whiteStyle, 0);
    lv_style_set_radius(&whiteStyle, 0);
    lv_style_set_border_width(&whiteStyle, 1);
    lv_style_set_border_color(&whiteStyle, GRAY_COLOR);
    lv_style_set_bg_color(&whiteStyle, WHITE_COLOR);

    lv_style_init(&blackStyle);
    lv_style_copy(&blackStyle, &whiteStyle);
    lv_style_set_bg_color(&blackStyle, BLACK_COLOR);

    _octave = 0;

    int posx = 0;
    for(int i=0; i<WHITE_KEYS_COUNT; ++i) {
        Key *k = new Key(this, _whiteLetters[i%7]);
        k->setSize(WHITE_WIDTH, height);
        k->setPos(posx, 0);
        k->setTextColorHex(0x000000);
        k->setTextPos(0, 170);
        k->setFont(&DEFAULT_FONT);
        lv_obj_add_style(k->lvhost(), &whiteStyle, 0);
        
        posx += WHITE_WIDTH;

        k->note = _defaultWhiteNotes[i];        
        k->setCallback([k, this]() {
            LOG_INFO("Key pressed: %d", (k->note + this->octaveModifier()));
            slr::Events::VirtualMidiKbdAction e = {
                .note = k->note + this->octaveModifier(),
                .velocity = 127,
                .channel = 0,
                .isPressed = true
            };
            slr::EmitEvent(e);
        }); 
        k->setTouchUpCallback([k, this]() {
            k->setColor(WHITE_COLOR);
            slr::Events::VirtualMidiKbdAction e = {
                .note = k->note + this->octaveModifier(),
                .velocity = 127,
                .channel = 0,
                .isPressed = false
            };
            slr::EmitEvent(e);
        });

        _keys.push_back(k);
    }

    int tmp = WHITE_WIDTH/3;
    posx = WHITE_WIDTH - tmp;
    for(int i=0; i<BLACK_KEYS_COUNT; ++i) {
        Key *k = new Key(this);
        k->setSize(BLACK_WIDTH, height - (height/3));
        k->setPos(posx, 0);
        lv_obj_add_style(k->lvhost(), &blackStyle, 0);

        int tmp2 = i%5;
        if(tmp2 == 1 || tmp2 == 4) {
            posx += (WHITE_WIDTH*2);
        } else {
            posx += WHITE_WIDTH;
        }

        k->note = _defaultBlackNotes[i];
        k->setCallback([k, this]() {
            LOG_INFO("Key pressed: %d", (k->note + this->octaveModifier()));
        });
        k->setTouchUpCallback([k]() {
            k->setColor(BLACK_COLOR);
        });
        
        _keys.push_back(k);
    }
}

VirtualMidiKeyboard::~VirtualMidiKeyboard() {
    for(auto k : _keys) {
        delete k;
    }
}

VirtualMidiKeyboard::Key::Key(BaseWidget *parent, std::string text) :
    Button(parent, text)
{

}

VirtualMidiKeyboard::Key::~Key() {

}

}