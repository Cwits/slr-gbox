// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "units/SimpleOscillator/SimpleOscUnit.h"

#include "core/primitives/ClipContainer.h"

#include "display/primitives/UnitUIBase.h"
#include "push/primitives/UnitUIBase.h"

#include "units/SimpleOscillator/SimpleOsc.h"
#include "units/SimpleOscillator/SimpleOscUI.h"
#include "units/SimpleOscillator/SimpleOscView.h"
#include "units/SimpleOscillator/SimpleOscPushUI.h"

#include <memory>

std::unique_ptr<slr::AudioUnit> createSimpleOscRT(const slr::ClipContainer *initContainer, const slr::ID forcedId) { 
    return std::make_unique<slr::SimpleOsc>(initContainer, forcedId);
}

std::shared_ptr<slr::AudioUnitView> createSimpleOscView(slr::AudioUnit * osc) {
    return std::make_shared<slr::SimpleOscView>(static_cast<slr::SimpleOsc*>(osc));
}

std::unique_ptr<UI::UnitUIBase> createSimpleOscUI(const std::shared_ptr<const slr::AudioUnitView> &osc, UI::UIContext * uictx) {
    return std::make_unique<UI::SimpleOscUI>(osc, uictx);
}

std::unique_ptr<PushUI::UnitUIBase> createSimpleOscPushUI(const std::shared_ptr<const slr::AudioUnitView> &osc, PushUI::PushUIContext * uictx) {
    return std::make_unique<PushUI::SimpleOscPushUI>(osc, uictx);
}

const std::string_view _simpleOscName = "SimpleOSC";

const slr::UnitDescriptor SimpleOscDescriptor {
    ._name = &_simpleOscName,
    .createRT = createSimpleOscRT,
    .createView = createSimpleOscView,
    .createUI = createSimpleOscUI,
    .createPushUI = createSimpleOscPushUI
};


