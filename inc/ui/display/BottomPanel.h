// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/display/primitives/View.h"
// #include "ui/display/primitives/Button.h"
// #include "ui/display/primitives/Label.h"

namespace UI {

class Button;
class Label;
class UIContext;

struct BottomPanel : public View {
    BottomPanel(BaseWidget * parent, UIContext * const uictx);
    ~BottomPanel();

    void updateTimelineRelated(const bool timeSigOrBpm);
    void update() override {}
    // private:
    Button * _newButton;
    Button * _playButton;
    Button * _stopButton;
    Button * _recButton;
    Button * _loopButton;

    Label * _posText;
    Label * _testPlayhead;
    
    Label * _bpmText;
    Label * _barSizeText;

    Label * _loopStartText;
    Label * _loopStart;
    Label * _loopEndText;
    Label * _loopEnd;
};

}