// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lvgl.h"
#include "ui/display/primitives/View.h"

namespace UI {

class Button;
class Label;
class UIContext;

class TopPanel : public View {
    public:
    TopPanel(BaseWidget * parent, UIContext * const uictx);
    ~TopPanel();

    void setMetroColor(lv_color_t color);
    void update() override {}
    
    private:
    Label * _lblProjectName;

    Button * _btnGrid;
    Button * _btnTrack;
    Button * _btnBrowser;
    Button * _btnMixer;
    Button * _btnStepSequencer;
    Button * _btnModEngine;
    Button * _btnSettings;
    Button * _btnToggleMetronome;

};

}