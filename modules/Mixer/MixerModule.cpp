// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modules/Mixer/MixerModule.h"

#include "core/ModuleManager.h"
#include "core/primitives/ClipContainer.h"

#include "ui/display/primitives/UnitUIBase.h"
#include "ui/push/primitives/UnitUIBase.h"

#include "modules/Mixer/Mixer.h"
#include "modules/Mixer/MixerUI.h"
#include "modules/Mixer/MixerView.h"

#include <memory>

std::unique_ptr<slr::AudioUnit> createMixerRT(const slr::ClipContainer * initContainer) { 
    return std::make_unique<slr::Mixer>(initContainer);
}

std::unique_ptr<slr::AudioUnitView> createMixerView(slr::AudioUnit * mixer) {
    return std::make_unique<slr::MixerView>(static_cast<slr::Mixer*>(mixer));
}

UI::UnitUIBase * createMixerUI(slr::AudioUnitView * mixer, UI::UIContext * uictx) {
    return new UI::MixerUI(mixer, uictx);
}

std::unique_ptr<PushUI::UnitUIBase> createMixerPushUI(slr::AudioUnitView * mixer, PushUI::PushUIContext * uictx) {
    // return std::make_unique<PushUI::MixerPushUI>(mixer, uictx);
    return std::unique_ptr<PushUI::UnitUIBase>(nullptr);
}

const std::string_view _mixerName = "Mixer";

const slr::Module MixerModule {
    ._name = &_mixerName,
    ._type = slr::ModuleType::Basic,
    .createRT = createMixerRT,
    .createView = createMixerView,
    .createUI = createMixerUI,
    .createPushUI = createMixerPushUI
};


