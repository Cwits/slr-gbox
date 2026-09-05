// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display/elements/TopPanel.h"

#include "display/utility/layoutSizes.h"
#include "display/utility/defaultStyles.h"
#include "display/utility/UIContext.h"

#include "display/primitives/Label.h"
#include "display/primitives/Button.h"

#include "display/popups/PopupManager.h"

#include  "core/actions/Actions.h"

#include "common/logger.h"

namespace UI {

TopPanel::TopPanel(BaseWidget * parent, UIContext * const uictx) : View(parent, uictx) {
    setPos(0, 0);
    setSize(Layout::TOP_PANEL_WIDTH, Layout::TOP_PANEL_HEIGHT);
    //set style
    // lv_obj_add_style(_lvhost, &workspace, 0);
    // lv_obj_set_style_pad_all(_lvhost, DEFAULT_MARGIN, LV_PART_MAIN);
    lv_obj_set_style_bg_color(_lvhost, lv_palette_main(LV_PALETTE_INDIGO), LV_PART_MAIN);

    // _lb = lv_label_create(_lvhost);
    // lv_label_set_text(_lb, "Unnamed Project");
    // lv_obj_set_pos(_lb, 0, 10);
    // lv_obj_set_style_text_color(_lb, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    // lv_obj_set_style_text_font(_lb, &lv_font_montserrat_40, 0);

    _lblProjectName = std::make_unique<Label>(this, "Untitled Project");
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

    _btnSave = std::make_unique<Button>(this, "Save");
    _btnSave->setPos(450, 0);
    _btnSave->setSize(Layout::Button, Layout::Button);
    _btnSave->setFont(&DEFAULT_FONT);
    _btnSave->setCallback([]() {
        auto act = std::make_unique<slr::Actions::SaveProject>();
        slr::EmitAction(std::move(act));
    });

    _btnLoad = std::make_unique<Button>(this, "Load");
    _btnLoad->setPos(450+Layout::Button+20, 0);
    _btnLoad->setSize(Layout::Button, Layout::Button);
    _btnLoad->setFont(&DEFAULT_FONT);
    _btnLoad->setCallback([]() {
        // auto act = std::make_unique<slr::Actions::SaveProject>();
        // slr::EmitAction(std::move(act));
        LOG_WARN("Not ready");
    });
    

    int posx = 750;
    _btnGrid = std::make_unique<Button>(this, "Grid");
    _btnGrid->setPos(posx, 0);
    _btnGrid->setSize(Layout::Button, Layout::Button);
    _btnGrid->setFont(&DEFAULT_FONT);
    _btnGrid->setCallback([uictx = _uictx]() {
        uictx->switchToView(MainView::Grid);
    });

    posx += (Layout::Margin + Layout::Button);
    _btnTrack = std::make_unique<Button>(this, "Unit");
    _btnTrack->setPos(posx, 0);
    _btnTrack->setSize(Layout::Button, Layout::Button);
    _btnTrack->setFont(&DEFAULT_FONT);
    _btnTrack->setCallback([uictx = _uictx]() {
        uictx->switchToView(MainView::Unit);
    });
    
    posx += (Layout::Margin + Layout::Button);
    _btnBrowser = std::make_unique<Button>(this, LV_SYMBOL_FILE);
    _btnBrowser->setPos(posx, 0);
    _btnBrowser->setSize(Layout::Button, Layout::Button);
    _btnBrowser->setFont(&DEFAULT_FONT);
    _btnBrowser->setCallback([uictx = _uictx]() {
        uictx->switchToView(MainView::Browser);
    });

    posx += (Layout::Margin + Layout::Button);
    _btnStepSequencer = std::make_unique<Button>(this, "StepS");
    _btnStepSequencer->setPos(posx, 0);
    _btnStepSequencer->setSize(Layout::Button, Layout::Button);
    _btnStepSequencer->setFont(&DEFAULT_FONT);
    _btnStepSequencer->setCallback([uictx = _uictx]() {
        uictx->switchToView(MainView::StepSequencer);
    });

    posx += (Layout::Margin + Layout::Button);
    _btnModEngine = std::make_unique<Button>(this, "ModE");
    _btnModEngine->setPos(posx, 0);
    _btnModEngine->setSize(Layout::Button, Layout::Button);
    _btnModEngine->setFont(&DEFAULT_FONT);
    _btnModEngine->setCallback([uictx = _uictx]() {
        // LOG_INFO("Modulation Engine will be added in future versions");
        uictx->switchToView(MainView::ModEngine);
    });
      
    posx = parent->width()-Layout::Button-Layout::Margin;
    _btnSettings = std::make_unique<Button>(this, LV_SYMBOL_LIST); //LV_SYMBOL_SETTINGS
    _btnSettings->setPos(posx, 0);
    _btnSettings->setSize(Layout::Button, Layout::Button);
    _btnSettings->setFont(&DEFAULT_FONT);
    _btnSettings->setCallback([this]() {
        this->_uictx->_popManager->enableSettingsPopup();
    });

    posx -= (Layout::Button+Layout::Margin);
    _btnToggleMetronome = std::make_unique<Button>(this, LV_SYMBOL_BELL);
    _btnToggleMetronome->setPos(posx, 0);
    _btnToggleMetronome->setSize(Layout::Button, Layout::Button);
    _btnToggleMetronome->setFont(&DEFAULT_FONT);
    _btnToggleMetronome->setCallback([]() {
        auto act = std::make_unique<slr::Actions::ToggleMetronome>();
        slr::EmitAction(std::move(act));
    });

    posx -= (Layout::Button+Layout::Margin);
    _btnMidiKbd = std::make_unique<Button>(this, "MIDI Kbd");
    _btnMidiKbd->setPos(posx, 0);
    _btnMidiKbd->setSize(Layout::Button, Layout::Button);
    _btnMidiKbd->setFont(&DEFAULT_FONT);
    _btnMidiKbd->setCallback([this]() {
        this->_uictx->_popManager->enableMidiKeyboard();
    });

    posx -= (Layout::Button+Layout::Margin);
    _btnRedo = std::make_unique<Button>(this, LV_SYMBOL_RIGHT);
    _btnRedo->setPos(posx, 0);
    _btnRedo->setSize(Layout::Button, Layout::Button);
    _btnRedo->setFont(&DEFAULT_FONT);
    _btnRedo->setCallback([this]() {
        // LOG_INFO("Redo action");
        auto act = std::make_unique<slr::Actions::Redo>();
        slr::EmitAction(std::move(act));
    });

    posx -= (Layout::Button+Layout::Margin);
    _btnUndo = std::make_unique<Button>(this, LV_SYMBOL_LEFT);
    _btnUndo->setPos(posx, 0);
    _btnUndo->setSize(Layout::Button, Layout::Button);
    _btnUndo->setFont(&DEFAULT_FONT);
    _btnUndo->setCallback([this]() {
        // LOG_INFO("Undo action");
        auto act = std::make_unique<slr::Actions::Undo>();
        slr::EmitAction(std::move(act));
    });
    
    show();
}

TopPanel::~TopPanel() {
}

void TopPanel::setMetroColor(lv_color_t color) {
    _btnToggleMetronome->setColor(color);
}

}