// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/BottomPanel.h"

#include "ui/display/primitives/Button.h"
#include "ui/display/primitives/Label.h"
#include "ui/display/layoutSizes.h"
#include "ui/display/defaultColors.h"
#include "ui/display/primitives/UIContext.h"
#include "ui/uiutility.h"

#include "snapshots/TimelineView.h"

#include "core/Actions.h"
// #include "core/Events.h"

#include "logger.h"

namespace UI {

BottomPanel::BottomPanel(BaseWidget * parent, UIContext * const uictx) 
    : View(parent, uictx) 
{
    setPos(LayoutDef::BOTTOM_PANEL_X, LayoutDef::BOTTOM_PANEL_Y);
    setSize(LayoutDef::BOTTOM_PANEL_WIDTH, LayoutDef::BOTTOM_PANEL_HEIGHT);
    setColor(lv_palette_main(LV_PALETTE_TEAL));  

    int posy = 10;
    int posx = 10;

    _btnPlay = std::make_unique<Button>(this, LV_SYMBOL_PLAY);
    _btnPlay->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnPlay->setPos(posx, posy);
    _btnPlay->setFont(&DEFAULT_FONT);
    _btnPlay->setCallback([this]() { 
        // LOG_INFO("Play");
        slr::TimelineView & tl = slr::TimelineView::getTimelineView();
        auto act = std::make_unique<slr::Actions::ChangeTimelineState>();
        act->state = (tl.playing() ? slr::TimelineState::Pause : slr::TimelineState::Play);
        slr::EmitAction(std::move(act));
    });
    
    posx += (LayoutDef::BUTTON_SIZE + LayoutDef::DEFAULT_MARGIN);
    _btnStop = std::make_unique<Button>(this, LV_SYMBOL_STOP);
    _btnStop->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnStop->setPos(posx, posy);
    _btnStop->setFont(&DEFAULT_FONT);
    _btnStop->setCallback([this]() { 
        // LOG_INFO("Stop");
        
        slr::TimelineView & tl = slr::TimelineView::getTimelineView();
        auto act = std::make_unique<slr::Actions::ChangeTimelineState>();
        act->state = slr::TimelineState::Stop;
        slr::EmitAction(std::move(act));
    });

    posx += (LayoutDef::BUTTON_SIZE + LayoutDef::DEFAULT_MARGIN);
    _btnRec = std::make_unique<Button>(this, LV_SYMBOL_EDIT);
    _btnRec->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnRec->setPos(posx, posy);
    _btnRec->setFont(&DEFAULT_FONT);
    _btnRec->setCallback([this]() { 
        // LOG_INFO("Record");
        slr::TimelineView & tl = slr::TimelineView::getTimelineView();
        auto act = std::make_unique<slr::Actions::ChangeTimelineState>();
        act->state = (tl.recording() ? slr::TimelineState::StopRecord : slr::TimelineState::StartRecord);
        slr::EmitAction(std::move(act));
    });

    posx += (LayoutDef::BUTTON_SIZE + LayoutDef::DEFAULT_MARGIN);
    _btnLoop = std::make_unique<Button>(this, LV_SYMBOL_LOOP);
    _btnLoop->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnLoop->setPos(posx, posy);
    _btnLoop->setFont(&DEFAULT_FONT);
    _btnLoop->setCallback([this]() { 
        // LOG_INFO("Toggle loop");
        slr::TimelineView &tl = slr::TimelineView::getTimelineView();
        auto act = std::make_unique<slr::Actions::ToggleLoop>();
        act->newState = (tl.looping() ? false : true);
        slr::EmitAction(std::move(act));
    });

    slr::TimelineView & tl = slr::TimelineView::getTimelineView();

    _lblBpmText = std::make_unique<Label>(this, UIUtility::bpmToString(tl.bpm()));
    _lblBpmText->setSize(200, lv_font_get_line_height(&DEFAULT_FONT));
    _lblBpmText->setFont(&DEFAULT_FONT);
    _lblBpmText->setPos(450, 5);
    _lblBpmText->setTapCallback([this]() {
        this->_uictx->_popManager->enableKeyboard(
            this->_lblBpmText->text(),
            [uictx = this->_uictx](const std::string &text) {
                //TODO: extract validation to separate UIUtility function ( bpm = validateBpm(string, uictx); )          
                float bpm = UIUtility::stringToBpm(text);
                if(bpm == -1) {
                    LOG_ERROR("Wrong format or value for BPM. %s to bpm %f", text.c_str(), bpm);
                    std::string error = text;
                    error.append(" is wrong format or value for BPM, correct is \'120.00\'");
                    uictx->floatingText(true, error);
                    return;
                }

                if(bpm < 1.0f) {
                    LOG_WARN("Minimal BPM is 1.0f, tried: %f", bpm);
                    std::string error = "Minimal BPM is 1.00";
                    uictx->floatingText(true, error);
                    bpm = 1.0f;
                }

                if(bpm > 400.0f) {
                    LOG_WARN("Maximal BPM is 400.0f, tried: %f", bpm);
                    std::string error = "Maximal BPM is 400.00";
                    uictx->floatingText(true, error);
                    bpm = 400.0f;
                }

                LOG_INFO("BPM set text: %s, res value: %f", text.c_str(), bpm);
                //event
                // LOG_WARN("No event to change bpm");
                slr::TimelineView &tl = slr::TimelineView::getTimelineView();
                auto act = std::make_unique<slr::Actions::ChangeSignatureBpm>();
                act->bpm = bpm;
                act->sig = tl.getBarSize();
                slr::EmitAction(std::move(act));
            }
        );
    });

    _lblBarSizeText = std::make_unique<Label>(this);
    _lblBarSizeText->setSize(200, lv_font_get_line_height(&DEFAULT_FONT));
    _lblBarSizeText->setFont(&DEFAULT_FONT);
    _lblBarSizeText->setPos(450, 55);
    _lblBarSizeText->setText(UIUtility::signatureToString(tl.getBarSize()));
    _lblBarSizeText->setTapCallback([this]() {
        this->_uictx->_popManager->enableKeyboard(
            this->_lblBarSizeText->text(),
            [uictx = this->_uictx](const std::string &text) {
                //TODO: extract validation to separate UIUtility function ( bpm = validateBarSize(string, uictx); )
                slr::BarSize size = UIUtility::stringToSig(text);

                if(size._numerator == 0 || size._denominator == 0) {
                    LOG_ERROR("Wrong format or value for Time Signature. %s", text.c_str());
                    std::string error = text;
                    error.append(" is wrong format or value for Time Signature, correct is \'4/4\'");
                    uictx->floatingText(true, error);
                    return;
                }

                if(size._numerator > 128) {
                    LOG_WARN("Time Signature Numenator is not allowed to be bigger than 128. %u", size._numerator);
                    uictx->floatingText(true, "Time Signature Numenator is not allowed to be bigger than 128.");
                    return;
                }

                if(size._denominator > 128) {
                    LOG_WARN("Time Signature Denominator is not allowed to be bigger than 128. %u", size._denominator);
                    uictx->floatingText(true, "Time Signature Denominator is not allowed to be bigger than 128.");
                    return;
                }

                LOG_INFO("Time signature set text: %s, val: %u/%u", text.c_str(), size._numerator, size._denominator);
                //event
                slr::TimelineView &tl = slr::TimelineView::getTimelineView();
                auto act = std::make_unique<slr::Actions::ChangeSignatureBpm>();
                act->bpm = tl.bpm();
                act->sig = size;
                slr::EmitAction(std::move(act));
            }
        );
    });

    //loop
    _lblLoopStartText = std::make_unique<Label>(this, "Loop Start");
    _lblLoopStartText->setSize(200, lv_font_get_line_height(&DEFAULT_FONT));
    _lblLoopStartText->setPos(600, 5);
    _lblLoopStartText->setFont(&DEFAULT_FONT);
    
    _lblLoopEndText = std::make_unique<Label>(this, "Loop End");
    _lblLoopEndText->setSize(200, lv_font_get_line_height(&DEFAULT_FONT));
    _lblLoopEndText->setPos(600, 55);
    _lblLoopEndText->setFont(&DEFAULT_FONT);

    _lblLoopStart = std::make_unique<Label>(this, std::to_string(tl.loopStartFrame()));
    _lblLoopStart->setSize(200, lv_font_get_line_height(&DEFAULT_FONT));
    _lblLoopStart->setPos(800, 5);
    _lblLoopStart->setFont(&DEFAULT_FONT);
    _lblLoopStart->setTapCallback([this]() {
        this->_uictx->_popManager->enableKeyboard(
            this->_lblLoopStart->text(),
            [uictx = this->_uictx](const std::string &text) {          
                slr::TimelineView &tl = slr::TimelineView::getTimelineView();
                bool failed = false;
                slr::frame_t start = UIUtility::textToLoop(text, tl.framesPerBar(), tl.framesPerQuater(), failed);
                slr::frame_t end = tl.loopEndFrame();

                if(failed) {
                    LOG_WARN("Failed to convert loop start text %s to value %lu", text.c_str(), start);
                    std::string error = "Failed to convert";
                    error.append(text);
                    uictx->floatingText(true, error);
                    return;
                }

                if(start > end) {
                    LOG_WARN("Loop start %lu position can't be bigger than loop end %lu position", start, end);
                    start = end;
                }

                LOG_INFO("Loop start set text: %s, res value: %lu", text.c_str(), start);
                //event
                auto act = std::make_unique<slr::Actions::LoopPosition>();
                act->start = start;
                act->end = end;
                slr::EmitAction(std::move(act));
            }
        );
    });

    _lblLoopEnd = std::make_unique<Label>(this, std::to_string(tl.loopEndFrame()));
    _lblLoopEnd->setSize(200, lv_font_get_line_height(&DEFAULT_FONT));
    _lblLoopEnd->setPos(800, 55);
    _lblLoopEnd->setFont(&DEFAULT_FONT);
    _lblLoopEnd->setTapCallback([this]() {
        this->_uictx->_popManager->enableKeyboard(
            this->_lblLoopEnd->text(),
            [uictx = this->_uictx](const std::string &text) {          
                slr::TimelineView &tl = slr::TimelineView::getTimelineView();
                bool failed = false;
                slr::frame_t start = tl.loopStartFrame();
                slr::frame_t end = UIUtility::textToLoop(text, tl.framesPerBar(), tl.framesPerQuater(), failed);

                if(failed) {
                    LOG_WARN("Failed to convert loop end text %s to value %lu", text.c_str(), end);
                    std::string error = "Failed to convert";
                    error.append(text);
                    uictx->floatingText(true, error);
                    return;
                }

                if(end < start) {
                    LOG_WARN("Loop end %lu position can't be smaller than loop start %lu position", end, start);
                    end = start;
                }

                LOG_INFO("Loop end set text: %s, res value: %lu", text.c_str(), end);
                //event
                auto act = std::make_unique<slr::Actions::LoopPosition>();
                act->start = start;
                act->end = end;
                slr::EmitAction(std::move(act));
            }
        );
    });

    //frames
    _lblPosText = std::make_unique<Label>(this, "frames: ");
    _lblPosText->setSize(150, lv_font_get_line_height(&DEFAULT_FONT));
    _lblPosText->setPos(1000, 5);
    _lblPosText->setFont(&DEFAULT_FONT);

    _lblTestPlayhead = std::make_unique<Label>(this, "0");
    _lblTestPlayhead->setSize(200, lv_font_get_line_height(&DEFAULT_FONT));
    _lblTestPlayhead->setPos(1120, 5); //120 diff
    _lblTestPlayhead->setFont(&DEFAULT_FONT);

    _btnNewUnit = std::make_unique<Button>(this, LV_SYMBOL_PLUS);
    _btnNewUnit->setPos(LayoutDef::BOTTOM_PANEL_WIDTH-LayoutDef::BUTTON_SIZE-LayoutDef::DEFAULT_MARGIN, posy);
    _btnNewUnit->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnNewUnit->setFont(&DEFAULT_FONT);
    _btnNewUnit->setCallback([uictx = _uictx]() {
        uictx->_popManager->enableNewUnitPopup();
    });

    show();
}

BottomPanel::~BottomPanel() {
}

void BottomPanel::updateTimelineRelated(const bool timeSigOrBpm) {
    slr::TimelineView &tl = slr::TimelineView::getTimelineView();

    if(timeSigOrBpm) {
        _lblBpmText->setText(UIUtility::bpmToString(tl.bpm()));
        _lblBarSizeText->setText(UIUtility::signatureToString(tl.getBarSize()));
        //update bpm
        //update barsize
    }

    switch(tl.state()) {
        case(slr::Timeline::RollState::Play): {
            _btnPlay->setColor(PLAY_ON_COLOR);
            _btnPlay->setText(LV_SYMBOL_PAUSE);
        } break;
        case(slr::Timeline::RollState::Pause): {
            _btnPlay->setColor(PLAY_PAUSE_COLOR);
            _btnPlay->setText(LV_SYMBOL_PLAY);
        } break;
        case(slr::Timeline::RollState::Stop): {
            _btnPlay->setColor(BUTTON_DEFAULT_COLOR);
            _btnPlay->setText(LV_SYMBOL_PLAY);
        }
        case(slr::Timeline::RollState::Preparing): break;
    }

    if(tl.recording()) {
        _btnRec->setColor(REC_ON_COLOR);
    } else {
        _btnRec->setColor(BUTTON_DEFAULT_COLOR);
    }

    if(tl.looping()) {
        _btnLoop->setColor(LOOP_ON_COLOR);
    } else {
        _btnLoop->setColor(BUTTON_DEFAULT_COLOR);
    }

    
    _lblLoopStart->setText(std::to_string(tl.loopStartFrame()));
    _lblLoopEnd->setText(std::to_string(tl.loopEndFrame()));

    //...?
}

}