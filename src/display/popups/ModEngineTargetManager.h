// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "display/primitives/View.h"
#include "display/primitives/Popup.h"

#include "common/defines.h"

#include <memory>
#include <vector>
#include <array>

namespace UI {

struct Label;
struct Button;
struct DropDown;
struct MainWindow;
struct ModEngineView;

struct ModEngineTargetManager : public Popup {
    ModEngineTargetManager(BaseWidget *parent, ModEngineView *sParent, UIContext * const uictx);
    ~ModEngineTargetManager();

    void update();

    private:
    ModEngineView * _view;

    slr::ID _currentModID = 0;

    std::unique_ptr<Label> _lblCurrentMod; //e.g. "Modulation 1"
    std::unique_ptr<Button> _btnNextMod;
    std::unique_ptr<Button> _btnPrevMod;

    struct Targets {
        std::unique_ptr<Label> _lblName;
        std::unique_ptr<Label> _lblParameter;
        std::unique_ptr<Label> _lblAmmount;
        std::unique_ptr<Button> _btnDeleteTarget;
    };
 
    std::vector<Targets> _targets;

    std::unique_ptr<DropDown> _ddTargetSelector;
    std::unique_ptr<DropDown> _ddParameterSelector;
    std::unique_ptr<Label> _lblAmmount;
    std::unique_ptr<Button> _btnAddTarget;
};

}