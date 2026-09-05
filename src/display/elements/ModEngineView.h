// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "display/primitives/View.h"
#include "display/primitives/Popup.h"

#include "common/defines.h"

#include <memory>
#include <vector>
#include <array>

namespace slr {
    struct ModulationPatternView;
}

namespace UI {

struct Label;
struct Button;
struct DropDown;
struct MainWindow;
struct ModEngineTargetManager;

struct ModulationUI {
    ModulationUI(const std::shared_ptr<slr::ModulationPatternView> view);

    slr::ID id() const;
    // const std::array<slr::ID, slr::TARGET_COUNT> targetId() const;
    const slr::ModulationPatternView * view() const { return _view.get(); }

    // void pollUIUpdate() override;

    uint64_t _uiVersion;
    private:
    const std::shared_ptr<slr::ModulationPatternView> _view;

    // lv_obj_t * _cnvModView;
    // uint8_t * _drawBuffer;
};

struct ModEngineView : public View {
    ModEngineView(BaseWidget * parent, UIContext * const uictx);
    ~ModEngineView();
    
    void pollUIUpdate() override;

    void updateLine(slr::frame_t nudge);
    
    void showById(slr::ID id) {}
    
    void showByPos(std::size_t pos);
    // SequenceUI * currentSequence() const;
    // std::string currentMod() const; //temporary...

    ModulationUI * currentMod() const;
    void createModUI(const std::shared_ptr<slr::ModulationPatternView> view);

    private:
  
    // void updateUI();

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
    std::unique_ptr<Label> _lblPhaseText;
    std::unique_ptr<Label> _lblPhase;
    // std::unique_ptr<Label> _lblRandSeedText;
    // std::unique_ptr<Label> _lblRandSeed;
    // std::unique_ptr<Label> _lblLengthText;
    // std::unique_ptr<Label> _lblLength;
    // std::unique_ptr<Button> _btnLength; //toggles from beats to seconds/ms

    std::unique_ptr<Label> _lblMinimalText;
    std::unique_ptr<Label> _lblMinimal;
    std::unique_ptr<Label> _lblMaximalText;
    std::unique_ptr<Label> _lblMaximal;

    std::unique_ptr<Button> _btnTargetManager;

    std::size_t _currentSelectedMod;
    std::vector<std::unique_ptr<ModulationUI>> _modUIs;

    lv_obj_t * _line;
    lv_point_precise_t _points[2];

    lv_obj_t * _cnvModView;
    uint8_t * _drawBuffer;

    void redrawCanvas(const std::string type, float rate, float offset);
    float _tmpRate = 1.0f;
    float _tmpOffset = 0;
    int _tmpRandSeed = 0;

    ModEngineTargetManager *_tpop;
    friend class MainWindow;
    friend class ModEngineTargetManager;
};

}