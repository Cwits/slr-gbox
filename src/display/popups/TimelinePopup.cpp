// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display/popups/TimelinePopup.h"

#include "display/primitives/Button.h"
#include "display/primitives/Label.h"
#include "display/utility/layoutSizes.h"
#include "display/utility/DefaultStyles.h"
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

    _bpm = std::make_unique<Label>(this, "bpm");
    _bpm->setPos(50, 30);
    _bpm->setSize(200, lv_font_get_line_height(TLPOP_FONT));
    _bpm->setFont(TLPOP_FONT);

    _timeSignature = std::make_unique<Label>(this, "sig");
    _timeSignature->setPos(50, 300);
    _timeSignature->setSize(200, lv_font_get_line_height(TLPOP_FONT));
    _timeSignature->setFont(TLPOP_FONT);

    _applyBtn = std::make_unique<Button>(this);
    _applyBtn->setPos(Layout::TLPOP_APPLY_X, Layout::TLPOP_APPLY_Y);
    _applyBtn->setSize(Layout::Button, Layout::Button);
    _applyBtn->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        LOG_WARN("Update bpm and signature not implemented yet");
        //update timeline bpm and signature event

        this->hide();
        this->deactivate();
        return true;
    });

}

TimelinePopup::~TimelinePopup() {
}

void TimelinePopup::update() {
    slr::TimelineView &tl = slr::TimelineView::getTimelineView();
    _bpm->setText(UIUtility::bpmToString(tl.bpm()));
    _timeSignature->setText(UIUtility::signatureToString(tl.getBarSize()));
}

}