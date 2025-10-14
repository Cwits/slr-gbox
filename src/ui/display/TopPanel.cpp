// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/TopPanel.h"

#include "ui/display/layoutSizes.h"
#include "ui/display/defaultStyles.h"
#include "ui/display/primitives/UIContext.h"
#include "ui/display/primitives/Label.h"
#include "ui/display/primitives/Button.h"

#include "core/Events.h"

#include "logger.h"

namespace UI {

TopPanel::TopPanel(BaseWidget * parent, UIContext * const uictx) : View(parent, uictx) {
    setPos(0, 0);
    setSize(LayoutDef::TOP_PANEL_WIDTH, LayoutDef::TOP_PANEL_HEIGHT);
    //set style
    // lv_obj_add_style(_lvhost, &workspace, 0);
    // lv_obj_set_style_pad_all(_lvhost, DEFAULT_MARGIN, LV_PART_MAIN);
    lv_obj_set_style_bg_color(_lvhost, lv_palette_main(LV_PALETTE_INDIGO), LV_PART_MAIN);

    // _lb = lv_label_create(_lvhost);
    // lv_label_set_text(_lb, "Unnamed Project");
    // lv_obj_set_pos(_lb, 0, 10);
    // lv_obj_set_style_text_color(_lb, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    // lv_obj_set_style_text_font(_lb, &lv_font_montserrat_40, 0);

    _lblProjectName = new Label(this, "Untitled Project");
    _lblProjectName->setPos(10, 15);
    _lblProjectName->setSize(400, lv_font_get_line_height(&lv_font_montserrat_40));
    _lblProjectName->setTextColor(lv_color_hex(0xffffff));
    _lblProjectName->setFont(&lv_font_montserrat_40);
    _lblProjectName->setTapCallback([this]() {
        this->_uictx->_popManager->enableKeyboard(
            this->_lblProjectName->text(),
            [this](const std::string &text) {          
                this->_lblProjectName->setText(text);
                LOG_WARN("No event to update Project Name");
            }
        );
    });

    int posx = 600;
    _btnGrid = new Button(this, "Grid");
    _btnGrid->setPos(posx, 0);
    _btnGrid->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnGrid->setFont(&DEFAULT_FONT);
    _btnGrid->setCallback([uictx = _uictx]() {
        uictx->switchToView(MainView::Grid);
    });

    posx += (LayoutDef::DEFAULT_MARGIN + LayoutDef::BUTTON_SIZE);
    _btnTrack = new Button(this, "Last Unit");
    _btnTrack->setPos(posx, 0);
    _btnTrack->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnTrack->setFont(&DEFAULT_FONT);
    _btnTrack->setCallback([uictx = _uictx]() {
        uictx->switchToView(MainView::Module);
    });
    
    posx += (LayoutDef::DEFAULT_MARGIN + LayoutDef::BUTTON_SIZE);
    _btnBrowser = new Button(this, LV_SYMBOL_FILE);
    _btnBrowser->setPos(posx, 0);
    _btnBrowser->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnBrowser->setFont(&DEFAULT_FONT);
    _btnBrowser->setCallback([uictx = _uictx]() {
        uictx->switchToView(MainView::Browser);
    });

    posx += (LayoutDef::DEFAULT_MARGIN + LayoutDef::BUTTON_SIZE);
    _btnStepSequencer = new Button(this, "StepS");
    _btnStepSequencer->setPos(posx, 0);
    _btnStepSequencer->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnStepSequencer->setFont(&DEFAULT_FONT);
    _btnStepSequencer->setCallback([]() {
        LOG_INFO("Step Sequencer will be added in future versions");
    });

    posx += (LayoutDef::DEFAULT_MARGIN + LayoutDef::BUTTON_SIZE);
    _btnModEngine = new Button(this, "ModE");
    _btnModEngine->setPos(posx, 0);
    _btnModEngine->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnModEngine->setFont(&DEFAULT_FONT);
    _btnModEngine->setCallback([]() {
        LOG_INFO("Modulation Engine will be added in future versions");
    });
      
    posx = parent->width()-LayoutDef::BUTTON_SIZE-LayoutDef::DEFAULT_MARGIN;
    _btnSettings = new Button(this, LV_SYMBOL_SETTINGS);
    _btnSettings->setPos(posx, 0);
    _btnSettings->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnSettings->setFont(&DEFAULT_FONT);
    _btnSettings->setCallback([]() {

    });

    posx -= (LayoutDef::BUTTON_SIZE+LayoutDef::DEFAULT_MARGIN);
    _btnToggleMetronome = new Button(this, LV_SYMBOL_BELL);
    _btnToggleMetronome->setPos(posx, 0);
    _btnToggleMetronome->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnToggleMetronome->setFont(&DEFAULT_FONT);
    _btnToggleMetronome->setCallback([]() {
        // LOG_WARN("Metronome on/off not here yet");
        slr::Events::ToggleMetronome e;
        slr::EmitEvent(e);
    });


    show();
}

TopPanel::~TopPanel() {
    delete _lblProjectName;
    delete _btnGrid;
    delete _btnTrack;
    delete _btnBrowser;
    delete _btnStepSequencer;
    delete _btnModEngine;
    delete _btnSettings;
    delete _btnToggleMetronome;
}

void TopPanel::setMetroColor(lv_color_t color) {
    _btnToggleMetronome->setColor(color);
}

}