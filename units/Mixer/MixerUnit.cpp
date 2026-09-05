// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/Mixer/MixerUnit.h"

#include "core/primitives/ClipContainer.h"

#include "display/primitives/UnitUIBase.h"
#include "push/primitives/UnitUIBase.h"

#include "units/Mixer/Mixer.h"
#include "units/Mixer/MixerUI.h"
#include "units/Mixer/MixerView.h"
#include "units/Mixer/MixerPushUI.h"

#include <memory>

std::unique_ptr<slr::AudioUnit> createMixerRT(const slr::ClipContainer * initContainer, const slr::ID forcedId) { 
    return std::make_unique<slr::Mixer>(initContainer, forcedId);
}

std::shared_ptr<slr::AudioUnitView> createMixerView(slr::AudioUnit * mixer) {
    return std::make_shared<slr::MixerView>(static_cast<slr::Mixer*>(mixer));
}

std::unique_ptr<UI::UnitUIBase> createMixerUI(const std::shared_ptr<const slr::AudioUnitView> &mixer, UI::UIContext * uictx) {
    return std::make_unique<UI::MixerUI>(mixer, uictx);
}

std::unique_ptr<PushUI::UnitUIBase> createMixerPushUI(const std::shared_ptr<const slr::AudioUnitView> &mixer, PushUI::PushUIContext * uictx) {
    return std::make_unique<PushUI::MixerPushUI>(mixer, uictx);
}

const std::string_view _mixerName = "Mixer";

const slr::UnitDescriptor MixerDescriptor {
    ._name = &_mixerName,
    // ._type = slr::ModuleType::Basic,
    .createRT = createMixerRT,
    .createView = createMixerView,
    .createUI = createMixerUI,
    .createPushUI = createMixerPushUI
};


