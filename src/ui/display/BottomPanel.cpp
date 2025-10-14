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

#include "core/Events.h"

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

    _playButton = new Button(this, LV_SYMBOL_PLAY);
    _playButton->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _playButton->setPos(posx, posy);
    _playButton->setFont(&DEFAULT_FONT);
    _playButton->setCallback([this]() { 
        // LOG_INFO("Play");
        slr::TimelineView & tl = slr::TimelineView::getTimelineView();
        slr::Events::ChangeTimelineState e {
            .state = (tl.playing() ? slr::TimelineState::Pause : slr::TimelineState::Play)
        };
        slr::EmitEvent(e);
    });
    
    posx += (LayoutDef::BUTTON_SIZE + LayoutDef::DEFAULT_MARGIN);
    _stopButton = new Button(this, LV_SYMBOL_STOP);
    _stopButton->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _stopButton->setPos(posx, posy);
    _stopButton->setFont(&DEFAULT_FONT);
    _stopButton->setCallback([this]() { 
        // LOG_INFO("Stop");
        slr::Events::ChangeTimelineState e = {
            .state = slr::TimelineState::Stop
        };
        slr::EmitEvent(e);
    });

    posx += (LayoutDef::BUTTON_SIZE + LayoutDef::DEFAULT_MARGIN);
    _recButton = new Button(this, LV_SYMBOL_EDIT);
    _recButton->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _recButton->setPos(posx, posy);
    _recButton->setFont(&DEFAULT_FONT);
    _recButton->setCallback([this]() { 
        // LOG_INFO("Record");
        slr::TimelineView & tl = slr::TimelineView::getTimelineView();
        slr::Events::ChangeTimelineState e = {
            .state = (tl.recording() ? slr::TimelineState::StopRecord : slr::TimelineState::StartRecord)
        };
        slr::EmitEvent(e);
    });

    posx += (LayoutDef::BUTTON_SIZE + LayoutDef::DEFAULT_MARGIN);
    _loopButton = new Button(this, LV_SYMBOL_LOOP);
    _loopButton->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _loopButton->setPos(posx, posy);
    _loopButton->setFont(&DEFAULT_FONT);
    _loopButton->setCallback([this]() { 
        // LOG_INFO("Toggle loop");
        slr::TimelineView &tl = slr::TimelineView::getTimelineView();
        slr::Events::ToggleLoop e = {
            .newState = (tl.looping() ? false : true)
        };
        slr::EmitEvent(e);
    });

    slr::TimelineView & tl = slr::TimelineView::getTimelineView();

    _bpmText = new Label(this, UIUtility::bpmToString(tl.bpm()));
    _bpmText->setSize(200, lv_font_get_line_height(&DEFAULT_FONT));
    _bpmText->setFont(&DEFAULT_FONT);
    _bpmText->setPos(450, 5);
    _bpmText->setTapCallback([this]() {
        this->_uictx->_popManager->enableKeyboard(
            this->_bpmText->text(),
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
                slr::Events::ChangeSigBpm e = {
                    .bpm = bpm,
                    .sig = tl.getBarSize()
                }; 
                slr::EmitEvent(e);
            }
        );
    });

    _barSizeText = new Label(this);
    _barSizeText->setSize(200, lv_font_get_line_height(&DEFAULT_FONT));
    _barSizeText->setFont(&DEFAULT_FONT);
    _barSizeText->setPos(450, 55);
    _barSizeText->setText(UIUtility::signatureToString(tl.getBarSize()));
    _barSizeText->setTapCallback([this]() {
        this->_uictx->_popManager->enableKeyboard(
            this->_barSizeText->text(),
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
                slr::Events::ChangeSigBpm e = {
                    .bpm = tl.bpm(),
                    .sig = size
                }; 
                slr::EmitEvent(e);
            }
        );
    });

    //loop
    _loopStartText = new Label(this, "Loop Start");
    _loopStartText->setSize(200, lv_font_get_line_height(&DEFAULT_FONT));
    _loopStartText->setPos(600, 5);
    _loopStartText->setFont(&DEFAULT_FONT);
    
    _loopEndText = new Label(this, "Loop End");
    _loopEndText->setSize(200, lv_font_get_line_height(&DEFAULT_FONT));
    _loopEndText->setPos(600, 55);
    _loopEndText->setFont(&DEFAULT_FONT);

    _loopStart = new Label(this, std::to_string(tl.loopStartFrame()));
    _loopStart->setSize(200, lv_font_get_line_height(&DEFAULT_FONT));
    _loopStart->setPos(800, 5);
    _loopStart->setFont(&DEFAULT_FONT);
    _loopStart->setTapCallback([this]() {
        this->_uictx->_popManager->enableKeyboard(
            this->_loopStart->text(),
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
                slr::Events::LoopPosition e = { 
                    .start = start,
                    .end = end
                };
                slr::EmitEvent(e);
            }
        );
    });

    _loopEnd = new Label(this, std::to_string(tl.loopEndFrame()));
    _loopEnd->setSize(200, lv_font_get_line_height(&DEFAULT_FONT));
    _loopEnd->setPos(800, 55);
    _loopEnd->setFont(&DEFAULT_FONT);
    _loopEnd->setTapCallback([this]() {
        this->_uictx->_popManager->enableKeyboard(
            this->_loopEnd->text(),
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
                slr::Events::LoopPosition e = {
                    .start = start,
                    .end = end
                };
                slr::EmitEvent(e);
            }
        );
    });

    //frames
    _posText = new Label(this, "frames: ");
    _posText->setSize(150, lv_font_get_line_height(&DEFAULT_FONT));
    _posText->setPos(1000, 5);
    _posText->setFont(&DEFAULT_FONT);

    _testPlayhead = new Label(this, "0");
    _testPlayhead->setSize(200, lv_font_get_line_height(&DEFAULT_FONT));
    _testPlayhead->setPos(1120, 5); //120 diff
    _testPlayhead->setFont(&DEFAULT_FONT);

    _newButton = new Button(this, LV_SYMBOL_PLUS);
    _newButton->setPos(LayoutDef::BOTTOM_PANEL_WIDTH-LayoutDef::BUTTON_SIZE-LayoutDef::DEFAULT_MARGIN, posy);
    _newButton->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _newButton->setFont(&DEFAULT_FONT);
    _newButton->setCallback([uictx = _uictx]() {
        /*
        TODO future: opens popup window that allow to choose what to create:
        built-in units such as Track, Mixer, Sequence(for step sequencer), 
        Modulation(for modulation engine), Effect(built-in or external),
        or input/output nodes(for external in-outs patching - purely visual thing)
        */
        // LOG_INFO("Create New Track Event");
        // slr::Events::NewTrack e;
        // slr::EmitEvent(e); 
        uictx->_popManager->enableNewModulePopup();
    });

    show();
}

BottomPanel::~BottomPanel() {
    delete _playButton;
    delete _stopButton;
    delete _recButton;

    delete _loopButton;
    delete _testPlayhead;
    
    delete _loopStartText;
    delete _loopStart;
    delete _loopEndText;
    delete _loopEnd;
    
    delete _newButton;
}

void BottomPanel::updateTimelineRelated(const bool timeSigOrBpm) {
    slr::TimelineView &tl = slr::TimelineView::getTimelineView();

    if(timeSigOrBpm) {
        _bpmText->setText(UIUtility::bpmToString(tl.bpm()));
        _barSizeText->setText(UIUtility::signatureToString(tl.getBarSize()));
        //update bpm
        //update barsize
    }

    switch(tl.state()) {
        case(slr::Timeline::RollState::Play): {
            _playButton->setColor(PLAY_ON_COLOR);
            _playButton->setText(LV_SYMBOL_PAUSE);
        } break;
        case(slr::Timeline::RollState::Pause): {
            _playButton->setColor(PLAY_PAUSE_COLOR);
            _playButton->setText(LV_SYMBOL_PLAY);
        } break;
        case(slr::Timeline::RollState::Stop): {
            _playButton->setColor(BUTTON_DEFAULT_COLOR);
            _playButton->setText(LV_SYMBOL_PLAY);
        }
        case(slr::Timeline::RollState::Preparing): break;
    }

    if(tl.recording()) {
        _recButton->setColor(REC_ON_COLOR);
    } else {
        _recButton->setColor(BUTTON_DEFAULT_COLOR);
    }

    if(tl.looping()) {
        _loopButton->setColor(LOOP_ON_COLOR);
    } else {
        _loopButton->setColor(BUTTON_DEFAULT_COLOR);
    }

    
    _loopStart->setText(std::to_string(tl.loopStartFrame()));
    _loopEnd->setText(std::to_string(tl.loopEndFrame()));

    //...?
}

}