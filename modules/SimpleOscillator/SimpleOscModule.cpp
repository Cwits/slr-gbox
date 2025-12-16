// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "modules/SimpleOscillator/SimpleOscModule.h"

#include "core/ModuleManager.h"

#include "ui/display/primitives/UnitUIBase.h"

#include "modules/SimpleOscillator/SimpleOsc.h"
#include "modules/SimpleOscillator/SimpleOscUI.h"
#include "modules/SimpleOscillator/SimpleOscView.h"

#include <memory>

std::unique_ptr<slr::AudioUnit> createSimpleOscRT() { 
    return std::make_unique<slr::SimpleOsc>();
}

slr::AudioUnitView * createSimpleOscView(slr::AudioUnit * osc) {
    return new slr::SimpleOscView(static_cast<slr::SimpleOsc*>(osc));
}

UI::UnitUIBase * createSimpleOscUI(slr::AudioUnitView * osc, UI::UIContext * uictx) {
    return new UI::SimpleOscUI(osc, uictx);
}

std::string _simpleOscName = "OSC";

const slr::Module SimpleOscModule {
    ._name = &_simpleOscName,
    ._type = slr::ModuleType::Basic,
    .createRT = createSimpleOscRT,
    .createView = createSimpleOscView,
    .createUI = createSimpleOscUI
};


