// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/display/primitives/View.h"
#include "ui/display/primitives/Popup.h"

#include "defines.h"

#include <memory>
#include <vector>
#include <array>

namespace slr {
    struct SequenceView;
}

namespace UI {

struct Label;
struct Button;
struct DropDown;

struct SequenceUI {
    SequenceUI(const std::shared_ptr<slr::SequenceView> view);

    const slr::ID id() const;
    // const std::array<slr::ID, slr::TARGET_COUNT> targetId() const;
    const slr::SequenceView * view() const { return _view.get(); }

    // void pollUIUpdate() override;

    uint64_t _uiVersion;
    private:
    const std::shared_ptr<slr::SequenceView> _view;
    // const std::shared_ptr<slr::SequenceView> _view;
};

struct StepSequencerView : public View {
    StepSequencerView(BaseWidget * parent, UIContext * const uictx);
    ~StepSequencerView();
    void pollUIUpdate() override;

    void createSequenceUI(const std::shared_ptr<slr::SequenceView> view);
    
    void showById(slr::ID id) {}
    SequenceUI * currentSequence() const;

    private:
    /* First Line */
    std::unique_ptr<Label> _lblNumOfSequencesText;
    std::unique_ptr<Label> _lblNumOfSequences;
    std::unique_ptr<Button> _btnPrevSequence;
    std::unique_ptr<Button> _btnNextSequence;

    std::unique_ptr<Button> _btnNewSequence;
    std::unique_ptr<Button> _btnDeleteSequence;

    std::unique_ptr<Label> _lblModeText;
    std::unique_ptr<Button> _btnModeToggle; //Sequencing/Edit

    /* Second Line */
    
    std::unique_ptr<Label> _lblTotalStepsText;
    std::unique_ptr<Label> _lblTotalSteps;
    std::unique_ptr<Label> _lblStepLengthText;
    std::unique_ptr<Label> _lblStepLength;

    std::unique_ptr<Label> _lblPageText;
    std::unique_ptr<Label> _lblPage;
    
    std::unique_ptr<Button> _btnTargetManager;
    
    std::vector<std::unique_ptr<Button>> _pageButtons;
    std::unique_ptr<Button> _btnNewPage;
    std::unique_ptr<Button> _btnDeletePage;

    //actually it is Sequence Row
    struct SequenceLayer {
        void hide();
        void show();
        void moveY(int amount);
        std::array<std::unique_ptr<Button>, 16> _steps;
        std::unique_ptr<Button> _btnMute;
        std::unique_ptr<Label> _lblNote;

        bool _isActive;
    };

    struct LayersContainer : public BaseWidget {
        LayersContainer(StepSequencerView *parent);

        std::array<StepSequencerView::SequenceLayer, 16> _layerView;
        int activeLayers() const;
    
        std::unique_ptr<Button> _btnAddLayer;
        std::unique_ptr<Button> _btnRemoveLayer;

        private:
        StepSequencerView *_view;
        bool handleSwipe(GestLib::SwipeGesture & swipe) override;
    };

    LayersContainer _container;

    void showByPos(std::size_t pos);

    void switchPage(int idx);
    
    std::size_t _currentVisibleSeq;
    int _currentPage;
    std::vector<std::unique_ptr<SequenceUI>> _sequences;

    void updateUI();
    // bool handleSwipe(GestLib::SwipeGesture & swipe) override;
    // bool handleDrag(GestLib::DragGesture &drag) override;
};

struct TargetSelectPopup : public Popup {
    TargetSelectPopup(BaseWidget *parent, StepSequencerView *sParent, UIContext * const uictx);
    ~TargetSelectPopup();

    private:
    StepSequencerView * _view;

};

}