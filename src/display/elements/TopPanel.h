// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "display/primitives/View.h"

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
    std::unique_ptr<Label> _lblProjectName;

    std::unique_ptr<Button> _btnSave;
    std::unique_ptr<Button> _btnLoad;

    std::unique_ptr<Button> _btnGrid;
    std::unique_ptr<Button> _btnTrack;
    std::unique_ptr<Button> _btnBrowser;
    std::unique_ptr<Button> _btnMixer;
    std::unique_ptr<Button> _btnStepSequencer;
    std::unique_ptr<Button> _btnModEngine;
    
    std::unique_ptr<Button> _btnSettings;
    std::unique_ptr<Button> _btnToggleMetronome;
    std::unique_ptr<Button> _btnMidiKbd;
    std::unique_ptr<Button> _btnUndo;
    std::unique_ptr<Button> _btnRedo;

};

}