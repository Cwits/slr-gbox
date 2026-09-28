// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "display/primitives/View.h"

#include <string>
#include <vector>

namespace UI {

class Button;
class Label;
class UIContext;

class Browser : public View {
    public:
    Browser(BaseWidget* parent, UIContext * const uictx);
    ~Browser();

    void update() override {}

    private:
    void parse();
    void goUp();
    std::string _lastParsedPath;
    std::vector<std::string> _folders;
    std::vector<std::string> _files;
    
    std::string _dragndrop;

    Button * _refresh;
    Button * _dirUp;
    Label * _lastPath;

    struct element {
        element(){}
        ~element();
        void init(Browser *parent, std::string &iconText, std::string &labelText, int ypos);
        lv_obj_t * _icon;  
        Label * _text;
    };
    std::vector<element*> _elements;

    Label * _lastTouched;

    bool handleTouchDown(const GestLib::TouchDownEvent &down);
    bool handleTouchUp(const GestLib::TouchUpEvent &up);
    bool handleTap(const GestLib::TapGesture &tap);
    bool handleDrag(const GestLib::DragGesture & drag);
    bool handleSwipe(const GestLib::SwipeGesture &swipe);

    Label * findLabel(int x, int y);
};

}