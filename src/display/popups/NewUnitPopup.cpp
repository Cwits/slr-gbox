// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "display/popups/NewUnitPopup.h"

#include "display/utility/layoutSizes.h"
#include "display/primitives/Button.h"
#include "display/utility/DefaultStyles.h"
#include "display/utility/Macros.h"

#include  "core/actions/Actions.h"
#include "common/logger.h" 

namespace UI {

NewUnitPopup::NewUnitPopup(BaseWidget *parent, UIContext * const uictx) :
    Popup(parent, uictx)
{
    setSize(Layout::ROUTE_MANAGER_WIDTH, Layout::ROUTE_MANAGER_HEIGHT);
    setPos(Layout::ROUTE_MANAGER_X, Layout::ROUTE_MANAGER_Y);
    addStyle(&Style::PopupDefault);
    
    BUTTONDEF(_btnTrack, 100, 100, "Track");
    _btnTrack->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        auto action = std::make_unique<slr::Actions::CreateNewUnit>();
        action->name = "Track";
        slr::EmitAction(std::move(action));
        return true;
    });

    BUTTONDEF(_btnMixer, 300, 100, "Mixer");
    _btnMixer->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        auto action = std::make_unique<slr::Actions::CreateNewUnit>();
        action->name = "Mixer";
        slr::EmitAction(std::move(action));
        return true;
    });

    BUTTONDEF(_btnOsc, 500, 100, "OSC");
    _btnOsc->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        auto action = std::make_unique<slr::Actions::CreateNewUnit>();
        action->name = "SimpleOSC";
        slr::EmitAction(std::move(action));
        return true;
    });

    BUTTONDEF(_btnSampler, 100, 200, "Sampler");
    _btnSampler->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        auto action = std::make_unique<slr::Actions::CreateNewUnit>();
        action->name = "Sampler";
        slr::EmitAction(std::move(action));
        return true;
    });
}

NewUnitPopup::~NewUnitPopup() {
}

void NewUnitPopup::update() {

}


}