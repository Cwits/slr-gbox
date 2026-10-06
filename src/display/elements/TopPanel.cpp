// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display/elements/TopPanel.h"

#include "display/utility/layoutSizes.h"
#include "display/utility/DefaultStyles.h"
#include "display/utility/DefaultColors.h"
#include "display/utility/UIContext.h"
#include "display/utility/Macros.h"

#include "display/primitives/Label.h"
#include "display/primitives/Button.h"

#include "display/popups/PopupManager.h"

#include "core/actions/Actions.h"

#include "common/logger.h"

namespace UI {

TopPanel::TopPanel(BaseWidget * parent, UIContext * const uictx) : View(parent, uictx) {
    setPos(0, 0);
    setSize(Layout::TOP_PANEL_WIDTH, Layout::TOP_PANEL_HEIGHT);
    addStyle(&Style::Panels);

    _lblProjectName = std::make_unique<Label>(this, "Untitled Project");
    _lblProjectName->setPos(10, 15);
    _lblProjectName->setSize(400, lv_font_get_line_height(&lv_font_montserrat_40));
    _lblProjectName->setTextColor(lv_color_hex(0xffffff));
    _lblProjectName->setFont(&lv_font_montserrat_40);
    _lblProjectName->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        this->_uictx->_popManager->enableKeyboard(
            this->_lblProjectName->text(),
            [this](const std::string &text) {          
                this->_lblProjectName->setText(text);
                LOG_WARN("No event to update Project Name");
            }
        );
        return true;
    });

    BUTTONDEF(_btnSave, 450, 0, "Save");
    _btnSave->tapCallback([](const GestLib::TapGesture &tap) -> bool {
        auto act = std::make_unique<slr::Actions::SaveProject>();
        slr::EmitAction(std::move(act));
        return true;
    });

    BUTTONDEF(_btnLoad, 450+Layout::Button+20, 0, "Load");
    _btnLoad->tapCallback([](const GestLib::TapGesture &tap) -> bool {
        // auto act = std::make_unique<slr::Actions::SaveProject>();
        // slr::EmitAction(std::move(act));
        LOG_WARN("Not ready");
        return true;
    });
    

    int posx = 750;
    BUTTONDEF(_btnGrid, posx, 0, "Grid");
    _btnGrid->tapCallback([uictx = _uictx](const GestLib::TapGesture &tap) -> bool {
        uictx->switchToView(MainView::Grid);
        return true;
    });

    posx += (Layout::Margin + Layout::Button);
    BUTTONDEF(_btnTrack, posx, 0, "Unit");
    _btnTrack->tapCallback([uictx = _uictx](const GestLib::TapGesture &tap) -> bool {
        uictx->switchToView(MainView::Unit);
        return true;
    });
    
    posx += (Layout::Margin + Layout::Button);
    BUTTONDEF(_btnBrowser, posx, 0, LV_SYMBOL_FILE);
    _btnBrowser->tapCallback([uictx = _uictx](const GestLib::TapGesture &tap) -> bool {
        uictx->switchToView(MainView::Browser);
        return true;
    });

    posx += (Layout::Margin + Layout::Button);
    BUTTONDEF(_btnStepSequencer, posx, 0, "StepS");
    _btnStepSequencer->tapCallback([uictx = _uictx](const GestLib::TapGesture &tap) -> bool {
        uictx->switchToView(MainView::StepSequencer);
        return true;
    });

    posx += (Layout::Margin + Layout::Button);
    BUTTONDEF(_btnModEngine, posx, 0, "ModE");
    _btnModEngine->tapCallback([uictx = _uictx](const GestLib::TapGesture &tap) -> bool {
        // LOG_INFO("Modulation Engine will be added in future versions");
        uictx->switchToView(MainView::ModEngine);
        return true;
    });
      
    posx = parent->width()-Layout::Button-Layout::Margin;
    BUTTONDEF(_btnSettings, posx, 0, LV_SYMBOL_LIST);
    _btnSettings->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        this->_uictx->_popManager->enableSettingsPopup();
        return true;
    });

    posx -= (Layout::Button+Layout::Margin);
    BUTTONDEF(_btnToggleMetronome, posx, 0, LV_SYMBOL_BELL);
    _btnToggleMetronome->tapCallback([](const GestLib::TapGesture &tap) -> bool {
        auto act = std::make_unique<slr::Actions::ToggleMetronome>();
        slr::EmitAction(std::move(act));
        return true;
    });

    posx -= (Layout::Button+Layout::Margin);
    BUTTONDEF(_btnMidiKbd, posx, 0, "MIDI kbd");
    _btnMidiKbd->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        this->_uictx->_popManager->enableMidiKeyboard();
        return true;
    });

    posx -= (Layout::Button+Layout::Margin);
    BUTTONDEF(_btnRedo, posx, 0, LV_SYMBOL_RIGHT);
    _btnRedo->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        // LOG_INFO("Redo action");
        auto act = std::make_unique<slr::Actions::Redo>();
        slr::EmitAction(std::move(act));
        return true;
    });

    posx -= (Layout::Button+Layout::Margin);
    BUTTONDEF(_btnUndo, posx, 0, LV_SYMBOL_LEFT);
    _btnUndo->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        // LOG_INFO("Undo action");
        auto act = std::make_unique<slr::Actions::Undo>();
        slr::EmitAction(std::move(act));
        return true;
    });
    
    show();
}

TopPanel::~TopPanel() {
}

void TopPanel::setMetroColor(lv_color_t color) {
    _btnToggleMetronome->setColor(color);
}

}