// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modules/Mixer/MixerModule.h"

#include "core/ModuleManager.h"

#include "ui/display/primitives/UnitUIBase.h"

#include "modules/Mixer/Mixer.h"
#include "modules/Mixer/MixerUI.h"

#include <memory>

std::unique_ptr<slr::AudioUnit> createMixerRT() { 
    return std::make_unique<slr::Mixer>();
}

UI::UnitUIBase * createMixerUI(slr::AudioUnitView * mixer, UI::UIContext * uictx) {
    return new UI::MixerUI(mixer, uictx);
}

std::string _mixerName = "Mixer";

const slr::Module MixerModule {
    ._name = &_mixerName,
    ._type = slr::ModuleType::Basic,
    .createRT = createMixerRT,
    .createUI = createMixerUI
};


