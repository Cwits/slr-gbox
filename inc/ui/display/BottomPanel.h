// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/display/primitives/View.h"
#include <memory>

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
    std::unique_ptr<Button> _btnNewUnit;
    std::unique_ptr<Button> _btnPlay;
    std::unique_ptr<Button> _btnStop;
    std::unique_ptr<Button> _btnRec;
    std::unique_ptr<Button> _btnLoop;

    std::unique_ptr<Label> _lblPosText;
    std::unique_ptr<Label> _lblTestPlayhead;
    
    std::unique_ptr<Label> _lblBpmText;
    std::unique_ptr<Label> _lblBarSizeText;

    std::unique_ptr<Label> _lblLoopStartText;
    std::unique_ptr<Label> _lblLoopStart;
    std::unique_ptr<Label> _lblLoopEndText;
    std::unique_ptr<Label> _lblLoopEnd;
};

}