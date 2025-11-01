// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/primitives/Slider.h"
#include "ui/display/primitives/Label.h"
#include "ui/display/layoutSizes.h"
#include "logger.h"

namespace UI {


inline float map(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

const int minY = 20;
const int maxY = 600-70-20;

Slider::Slider(BaseWidget * parent) : 
    BaseWidget(parent, true)
{
    _flags.isDrag = true;
    setSize(200, 600);

    int posx = 200-50-20;
    int posy = 600-70-20;

    _sliderCap = lv_obj_create(lvhost());
    lv_obj_set_size(_sliderCap, 50, 70);
    lv_obj_set_pos(_sliderCap, posx, posy);
    lv_obj_set_style_bg_color(_sliderCap, lv_color_hex(0xacacac), 0);

    posy += (70/2);
    _lblMin = lv_label_create(lvhost());
    lv_obj_set_size(_lblMin, 50, lv_font_get_line_height(&DEFAULT_FONT));
    lv_obj_set_pos(_lblMin, posx-50, posy);
    lv_label_set_text(_lblMin, "0.0");
    lv_obj_set_style_text_font(_lblMin, &DEFAULT_FONT, 0);

    
    _lblMax = lv_label_create(lvhost());
    lv_obj_set_size(_lblMax, 50, lv_font_get_line_height(&DEFAULT_FONT));
    lv_obj_set_pos(_lblMax, posx-50, 20);
    lv_label_set_text(_lblMax, "1.5");
    lv_obj_set_style_text_font(_lblMax, &DEFAULT_FONT, 0);

    int correctY = map(1.0f, 0.0f, 1.5f, maxY, minY);

    _lblZero = lv_label_create(lvhost());
    lv_obj_set_size(_lblZero, 50, lv_font_get_line_height(&DEFAULT_FONT));
    lv_obj_set_pos(_lblZero, posx-50, correctY);
    lv_label_set_text(_lblZero, "1.0");
    lv_obj_set_style_text_font(_lblZero, &DEFAULT_FONT, 0);

    show();
}

Slider::~Slider() {
    lv_obj_delete(_sliderCap);
    lv_obj_delete(_lblMax);
    lv_obj_delete(_lblMin);
    lv_obj_delete(_lblZero);

}

void Slider::onChangeCallback(std::function<void(const float)> onChange) {
    _onChange = onChange;
}

void Slider::setCap(const float value) {
    int posy = map(value, 0.0f, 1.5f, maxY, minY);
    lv_obj_set_pos(_sliderCap, 200-50-20, posy);
}

const float Slider::getCap() {
    int posy = lv_obj_get_y(_sliderCap);
    float val = map(posy, maxY, minY, 0.0f, 1.5f);
    return val;
}

bool Slider::handleDrag(GestLib::DragGesture &drag) {
    int notAbsY = drag.y - getY();
    if(drag.state == GestLib::GestureState::Start) {

    } else if(drag.state == GestLib::GestureState::Move) {
        int posy = notAbsY-(70+20+(70/2));

        if(posy < minY) posy = minY;
        else if(posy > maxY) posy = maxY;
        
        lv_obj_set_pos(_sliderCap, 200-50-20, posy);

        float res = map(posy, maxY, minY, 0.0f, 1.5f);
        // LOG_INFO("res: %f", res);
        if(_onChange) {
            _onChange(res);
        }
    } else if(drag.state == GestLib::GestureState::End) {

    }

    return true;
}


}