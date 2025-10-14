// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/primitives/DropDown.h"
#include "ui/display/layoutSizes.h"
#include "logger.h"

#include <cmath>

namespace UI {

DropDown::DropDown(BaseWidget * parent) :
    BaseWidget(parent, true)
{
    _flags.isTap = true;
    _textColor = lv_color_hex(0x000000);
    _textFont = &lv_font_montserrat_40;
    lv_obj_set_style_clip_corner(lvhost(), false, 0);
    // lv_obj_set_style_bg_opa(lvhost(), LV_OPA_0, 0); // OPA_0 - 100% transparent, OPA_100 - 0% transparent

    _btn = new Button(parent);
    _btn->setText("No selection");
    _btn->setFont(_textFont);
    _btn->setCallback([this]() {
        if(this->_isClosed) this->open();
        else this->close();
    });

    _isClosed = true;
}

DropDown::~DropDown() {
    // if(_lastSelected) lv_obj_delete(_lastSelected);
    delete _btn;
    for(lv_obj_t *i : _items) {
        lv_obj_delete(i);
    }
}

void DropDown::setItems(std::vector<std::string> &items) {
    std::size_t size = items.size();
    std::size_t oldsize = _items.size();
    if(size == 0) {
        LOG_ERROR("Can't set up empty vector");
        return;
    }

    if(_items.size() < size) {
        _items.reserve(size);
    }

    for(std::size_t i=0; i<oldsize; ++i) {
        lv_obj_t *l = _items.at(i);
        lv_label_set_text(l, items.at(i).c_str());
    }

    lv_obj_update_layout(lvhost());

    int height = lv_font_get_line_height(_textFont);
    int width = this->width();

    int hostHeight = height*size;
    setSize(width, hostHeight);

    for(std::size_t i=oldsize; i<size; ++i) {
        lv_obj_t *l = lv_label_create(lvhost());
        lv_label_set_text(l, items.at(i).c_str());
        lv_obj_set_pos(l, 0, height*(i));
        lv_obj_set_size(l, width, height);
        lv_obj_set_style_text_color(l, _textColor, 0);
        lv_obj_set_style_text_font(l, _textFont, 0);
        _items.push_back(l);
    }

    // lv_label_set_text(_lastSelected, items.at(0).c_str());
    // lv_obj_set_size(_lastSelected, lv_obj_get_width(lvhost()), height);
    _btn->setText(items.at(0));
}

void DropDown::setTextColor(lv_color_t & color) {
    _textColor = color;
}

void DropDown::setTextFont(const lv_font_t * font) {
    _textFont = font;
}

void DropDown::selectedCallback(std::function<void(const std::string)> clb) {
    _callback = clb;
}

void DropDown::setSelected(std::string & item) {
    _btn->setText(item);
}

void DropDown::open() {
    _isClosed = false;
    lv_obj_clear_flag(lvhost(), LV_OBJ_FLAG_HIDDEN);
}

void DropDown::close() {
    _isClosed = true;
    lv_obj_add_flag(lvhost(), LV_OBJ_FLAG_HIDDEN);
}

bool DropDown::handleTap(GestLib::TapGesture & tap) {
    int height = lv_font_get_line_height(_textFont);
    std::size_t size = _items.size();
    
    //find item    
    int y = getY();
    int h = this->height();
    int index = ((tap.y-y) / height) - 1;
    if(index >= _items.size()) {
        LOG_ERROR("Wrong index");
        return false;
    }
    
    std::string text = std::string(lv_label_get_text(_items.at(index)));
        
    _btn->setText(text);
    if(_callback) _callback(text);

    close();
    
    return true;
}

}


/*
Второй вариант полностью на тебе. Если не используешь indev, LVGL не будет заниматься хитом тестом и выбором пункта.

Механика у lv_dropdown:

Список хранится внутри lv_dropdown как lv_obj_t * list = lv_dropdown_get_list(dropdown). Это обычный lv_list.

Каждый пункт — lv_button с лейблом.

Когда пункт жмётся стандартным способом, вызывается lv_dropdown_set_selected(dropdown, index) и потом шлётся LV_EVENT_VALUE_CHANGED.

Если обходишь indev, то единственное нормальное решение:

При своём клике по координатам открытого списка найти дочерний lv_obj под точкой через lv_obj_get_child(), lv_obj_get_coords(), и сверку с твоими (x, y).

Когда совпадение найдено, вызвать lv_dropdown_set_selected(dropdown, idx) и вручную lv_event_send(dropdown, LV_EVENT_VALUE_CHANGED, NULL).

Парсить размеры вручную не нужно — у каждого дочернего объекта есть lv_obj_get_coords(obj, &area). Хитом тестить именно этим, а не геометрию руками высчитывать.

Таким образом: открываешь lv_dropdown_open(), ждёшь клик в своей системе, ищешь ребёнка по координате, ставишь set_selected, и закрываешь lv_dropdown_close().
*/