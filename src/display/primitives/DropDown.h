// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "display/primitives/BaseWidget.h"

#include <functional>
#include <string>
#include <memory>

namespace UI {

struct Button;

struct DropDown : public BaseWidget {
    DropDown(BaseWidget * parent);
    ~DropDown();

    void setItems(std::vector<std::string> &items);
    void setSelected(const std::string_view item);
    void setSelected(std::string & item);
    // void setTextColor(lv_color_t &color);
    // void setTextFont(const lv_font_t *font);
    void selectedCallback(std::function<void(const std::string)> clb);
    const std::string selectedItem() const;

    void enableButton();
    void disableButton();
    bool isButtonDisabled() const;
    Button * button() const { return _btn.get(); }
    
    void setSize(lv_coord_t w, lv_coord_t h) override;
    void setPos(lv_coord_t x, lv_coord_t y) override;

    void close();
    private:
    std::unique_ptr<Button> _btn;
    std::vector<lv_obj_t*> _items;
    
    std::function<void(const std::string)> _callback;
    
    bool _isClosed;
    // const lv_font_t * _textFont;

    void open();

    bool handleTap(const GestLib::TapGesture & tap);
    
};

}