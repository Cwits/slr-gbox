// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/display/primitives/BaseWidget.h"
#include "ui/display/primitives/Button.h"

#include <functional>
#include <string>

namespace UI {

struct DropDown : public BaseWidget {
    DropDown(BaseWidget * parent);
    ~DropDown();

    void setItems(std::vector<std::string> &items);
    void setSelected(std::string & item);
    void setTextColor(lv_color_t &color);
    void setTextFont(const lv_font_t *font);
    void selectedCallback(std::function<void(const std::string)> clb);
    const std::string selectedItem() const { return _btn->text(); }

    void enableButton() { _btn->enable(); }
    void disableButton() { _btn->disable(); }
    bool isButtonDisabled() const { return _btn->isDisabled(); }
    Button * button() const { return _btn; }
    
    void setSize(lv_coord_t w, lv_coord_t h) override;
    void setPos(lv_coord_t x, lv_coord_t y) override;

    private:
    Button * _btn;
    std::vector<lv_obj_t*> _items;
    
    std::function<void(const std::string)> _callback;
    
    bool _isClosed;
    lv_color_t _textColor;
    const lv_font_t * _textFont;

    void open();
    void close();

    bool handleTap(GestLib::TapGesture & tap) override;
    
};

}