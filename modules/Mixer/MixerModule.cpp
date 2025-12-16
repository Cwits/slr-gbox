// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modules/Mixer/MixerModule.h"

#include "core/ModuleManager.h"

#include "ui/display/primitives/UnitUIBase.h"

#include "modules/Mixer/Mixer.h"
#include "modules/Mixer/MixerUI.h"
#include "modules/Mixer/MixerView.h"

#include <memory>

std::unique_ptr<slr::AudioUnit> createMixerRT() { 
    return std::make_unique<slr::Mixer>();
}

slr::AudioUnitView * createMixerView(slr::AudioUnit * mixer) {
    return new slr::MixerView(static_cast<slr::Mixer*>(mixer));
}

UI::UnitUIBase * createMixerUI(slr::AudioUnitView * mixer, UI::UIContext * uictx) {
    return new UI::MixerUI(mixer, uictx);
}

std::string _mixerName = "Mixer";

const slr::Module MixerModule {
    ._name = &_mixerName,
    ._type = slr::ModuleType::Basic,
    .createRT = createMixerRT,
    .createView = createMixerView,
    .createUI = createMixerUI
};


