// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display/primitives/Label.h"
#include "display/utility/layoutSizes.h"
#include "display/utility/DefaultStyles.h"
#include "common/logger.h"

namespace UI {

Label::Label(BaseWidget * parent, std::string text) :
    BaseWidget(parent, true)
{
    _label = lv_label_create(_lvhost);
    lv_label_set_text(_label, text.c_str());
    addStyle(&Style::Label);
    lv_obj_add_style(_label, &Style::Text, 0);
    // lv_obj_set_style_bg_opa(_lvhost, LV_OPA_TRANSP, 0);
    lv_obj_set_scrollbar_mode(_lvhost, LV_SCROLLBAR_MODE_OFF);
    // setFont(&DEFAULT_FONT);
    show();
}

Label::~Label() {
    lv_obj_delete(_label);
}

void Label::setText(std::string text) {
    lv_label_set_text(_label, text.c_str());
}

std::string Label::text() const {
    return std::string(lv_label_get_text(_label));
}

void Label::setTextPos(int x, int y) {
    lv_obj_set_pos(_label, x, y);
}

void Label::setTextColor(const lv_color_t color) {
    lv_obj_set_style_text_color(_label, color, 0);
}

void Label::setFont(const lv_font_t * font) {
    lv_obj_set_style_text_font(_label, font, 0);
}

}