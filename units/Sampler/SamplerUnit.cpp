// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/Sampler/SamplerUnit.h"

#include "core/primitives/ClipContainer.h"

#include "ui/display/primitives/UnitUIBase.h"
#include "ui/push/primitives/UnitUIBase.h"

#include "units/Sampler/Sampler.h"
#include "units/Sampler/SamplerUI.h"
#include "units/Sampler/SamplerView.h"
#include "units/Sampler/SamplerPushUI.h"

#include "units/Sampler/Actions/LoadAsset.h"

#include <memory>

std::unique_ptr<slr::AudioUnit> createSamplerRT(const slr::ClipContainer * initContainer, const slr::ID forcedId) { 
    return std::make_unique<slr::Sampler>(initContainer, forcedId);
}

std::shared_ptr<slr::AudioUnitView> createSamplerView(slr::AudioUnit * sampler) {
    return std::make_shared<slr::SamplerView>(static_cast<slr::Sampler*>(sampler));
}

std::unique_ptr<UI::UnitUIBase> createSamplerUI(const std::shared_ptr<const slr::AudioUnitView> &sampler, UI::UIContext * uictx) {
    return std::make_unique<UI::SamplerUI>(sampler, uictx);
}

std::unique_ptr<PushUI::UnitUIBase> createSamplerPushUI(const std::shared_ptr<const slr::AudioUnitView> &sampler, PushUI::PushUIContext * uictx) {
    return std::make_unique<PushUI::SamplerPushUI>(sampler, uictx);
}

const std::string_view _samplerName = "Sampler";

const slr::UnitDescriptor SamplerDescriptor {
    ._name = &_samplerName,
    .createRT = createSamplerRT,
    .createView = createSamplerView,
    .createUI = createSamplerUI,
    .createPushUI = createSamplerPushUI
};



namespace slr {

void registerSamplerActions(std::map<std::type_index, CreatorFn> &map) {
    map[typeid(Actions::LoadAsset)] = &createLoadAssetAction;
}

}