// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modules/Sampler/SamplerModule.h"

#include "core/ModuleManager.h"
#include "core/primitives/ClipContainer.h"

#include "ui/display/primitives/UnitUIBase.h"
#include "ui/push/primitives/UnitUIBase.h"

#include "modules/Sampler/Sampler.h"
#include "modules/Sampler/SamplerUI.h"
#include "modules/Sampler/SamplerView.h"
#include "modules/Sampler/SamplerPushUI.h"

#include <memory>

std::unique_ptr<slr::AudioUnit> createSamplerRT(const slr::ClipContainer * initContainer) { 
    return std::make_unique<slr::Sampler>(initContainer);
}

std::unique_ptr<slr::AudioUnitView> createSamplerView(slr::AudioUnit * sampler) {
    return std::make_unique<slr::SamplerView>(static_cast<slr::Sampler*>(sampler));
}

std::unique_ptr<UI::UnitUIBase> createSamplerUI(slr::AudioUnitView * sampler, UI::UIContext * uictx) {
    return std::make_unique<UI::SamplerUI>(sampler, uictx);
}

std::unique_ptr<PushUI::UnitUIBase> createSamplerPushUI(slr::AudioUnitView * sampler, PushUI::PushUIContext * uictx) {
    return std::make_unique<PushUI::SamplerPushUI>(sampler, uictx);
}

const std::string_view _samplerName = "Sampler";

const slr::Module SamplerModule {
    ._name = &_samplerName,
    ._type = slr::ModuleType::Basic,
    .createRT = createSamplerRT,
    .createView = createSamplerView,
    .createUI = createSamplerUI,
    .createPushUI = createSamplerPushUI
};


