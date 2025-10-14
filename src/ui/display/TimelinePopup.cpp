// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/TimelinePopup.h"

#include "ui/display/primitives/Button.h"
#include "ui/display/primitives/Label.h"
#include "ui/display/layoutSizes.h"
#include "ui/uiutility.h"

#include "snapshots/TimelineView.h"

#include "logger.h"

const lv_font_t * TLPOP_FONT = &DEFAULT_FONT;

namespace UI {

TimelinePopup::TimelinePopup(BaseWidget * parent, UIContext * const uictx) : 
    Popup(parent, uictx) 
{
    setSize(LayoutDef::TIMELINE_POPUP_W, LayoutDef::TIMELINE_POPUP_H);
    setPos(LayoutDef::TIMELINE_POPUP_X, LayoutDef::TIMELINE_POPUP_Y);
    setColor(lv_color_hex(0x4a5cf1));

    _bpm = new Label(this, "bpm");
    _bpm->setPos(50, 30);
    _bpm->setSize(200, lv_font_get_line_height(TLPOP_FONT));
    _bpm->setFont(TLPOP_FONT);

    _timeSignature = new Label(this, "sig");
    _timeSignature->setPos(50, 300);
    _timeSignature->setSize(200, lv_font_get_line_height(TLPOP_FONT));
    _timeSignature->setFont(TLPOP_FONT);

    _applyBtn = new Button(this);
    _applyBtn->setPos(LayoutDef::TLPOP_APPLY_X, LayoutDef::TLPOP_APPLY_Y);
    _applyBtn->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _applyBtn->setCallback([this]() {
        LOG_WARN("Update bpm and signature not implemented yet");
        //update timeline bpm and signature event

        this->hide();
        this->deactivate();
    });

}

TimelinePopup::~TimelinePopup() {
    delete _bpm;
    delete _timeSignature;
    delete _applyBtn;
}

void TimelinePopup::update() {
    slr::TimelineView &tl = slr::TimelineView::getTimelineView();
    _bpm->setText(UIUtility::bpmToString(tl.bpm()));
    _timeSignature->setText(UIUtility::signatureToString(tl.getBarSize()));
}

}