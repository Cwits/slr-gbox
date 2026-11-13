// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display/popups/TimelinePopup.h"

#include "display/primitives/Button.h"
#include "display/primitives/Label.h"
#include "display/primitives/Checkbox.h"

#include "display/utility/layoutSizes.h"
#include "display/utility/DefaultStyles.h"
#include "display/utility/UIContext.h"

#include "display/elements/Timeline.h"
#include "display/popups/PopupManager.h"

#include "common/uiutility.h"

#include "snapshots/TimelineView.h"

#include "common/logger.h"

const lv_font_t * TLPOP_FONT = &DEFAULT_FONT;

namespace UI {

TimelinePopup::TimelinePopup(BaseWidget * parent, UIContext * const uictx) : 
    Popup(parent, uictx) 
{
    setSize(Layout::TIMELINE_POPUP_W, Layout::TIMELINE_POPUP_H);
    setPos(Layout::TIMELINE_POPUP_X, Layout::TIMELINE_POPUP_Y);
    addStyle(&Style::PopupDefault);

    
    _lblGridSize = std::make_unique<Label>(this);
    _lblGridSize->setSize(Layout::TIMELINE_POPUP_W-10, 40);
    _lblGridSize->setPos(5, 10);
    _lblGridSize->tapCallback([this, uictx = uictx](const GestLib::TapGesture &tap) -> bool {
        uictx->_popManager->enableKeyboard(
            std::to_string(this->_current->gridSize()), 
            [this](const std::string &text) {
                int res = std::stoi(text);
                if(!this->_current) return;
                this->_current->gridSize(res);
                this->_current->rebuildTimeline();
            }
        );
        this->hide();
        this->deactivate();
        return true;
    });

    _chbTriplets = std::make_unique<Checkbox>(this);
    _chbTriplets->setSize(50, 50);
    _chbTriplets->setPos(30, 70);
    _chbTriplets->checkCallback([this](bool checked) {
        if(!this->_current) return;
        this->_current->triplets(checked);
        this->_current->rebuildTimeline();
        
    }); 

    _applyBtn = std::make_unique<Button>(this, "Apply");
    _applyBtn->setPos(Layout::TLPOP_APPLY_X, Layout::TLPOP_APPLY_Y);
    _applyBtn->setSize(Layout::Button, Layout::Button);
    _applyBtn->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        // LOG_WARN("Update bpm and signature not implemented yet");
        //update timeline bpm and signature event
        // this->_current->rebuildTimeline();

        this->hide();
        this->deactivate();
        return true;
    });

}

TimelinePopup::~TimelinePopup() {
}

void TimelinePopup::update() {
    if(!_current) return;

    _lblGridSize->setText(std::to_string(_current->gridSize()));
    _chbTriplets->setValue(_current->isTriplet());
}

}