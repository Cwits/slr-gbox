// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/display/primitives/View.h"
#include "ui/display/primitives/Popup.h"

#include "defines.h"

#include <memory>
#include <vector>
#include <array>

// namespace slr {
//     struct SequenceView;
// }

namespace UI {

struct Label;
struct Button;
struct DropDown;
struct MainWindow;

struct ModEngineView : public View {
    ModEngineView(BaseWidget * parent, UIContext * const uictx);
    ~ModEngineView();
    
    void pollUIUpdate() override {}

    void updateLine(slr::frame_t nudge);
    // void createSequenceUI(const std::shared_ptr<slr::SequenceView> view);
    
    void showById(slr::ID id) {}
    // SequenceUI * currentSequence() const;

    private:
  
    // void updateUI();

    friend class MainWindow; //ugh...
    // bool handleSwipe(GestLib::SwipeGesture & swipe) override;
    // bool handleDrag(GestLib::DragGesture &drag) override;

    std::unique_ptr<Label> _lblModNumText;
    std::unique_ptr<Label> _lblModNum;

    std::unique_ptr<Button> _btnNewMod;
    std::unique_ptr<Button> _btnDelMod;
    std::unique_ptr<Button> _btnNextMod;
    std::unique_ptr<Button> _btnPrevMod;

    std::unique_ptr<Label> _lblTypeText;
    std::unique_ptr<DropDown> _ddType; //sin, square, tri, saw, random
    std::unique_ptr<Label> _lblRateText;
    std::unique_ptr<Label> _lblRate;
    std::unique_ptr<Button> _btnRate; //toggles from Hz to beats and back
    std::unique_ptr<Label> _lblOffsetText;
    std::unique_ptr<Label> _lblOffset;
    // std::unique_ptr<Label> _lblRandSeedText;
    // std::unique_ptr<Label> _lblRandSeed;
    // std::unique_ptr<Label> _lblLengthText;
    // std::unique_ptr<Label> _lblLength;
    // std::unique_ptr<Button> _btnLength; //toggles from beats to seconds/ms

    std::unique_ptr<Button> _btnManageTargets;

    lv_obj_t * _line;
    lv_point_precise_t _points[2];

    lv_obj_t * _cnvModView;
    uint8_t * _drawBuffer;

    void redrawCanvas(const std::string type, float rate, float offset);
    float _tmpRate = 1.0f;
    float _tmpOffset = 0;
    int _tmpRandSeed = 0;
};

}