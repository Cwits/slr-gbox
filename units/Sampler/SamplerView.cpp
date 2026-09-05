// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "units/Sampler/SamplerView.h"
#include "units/Sampler/SamplerUnit.h"
#include "core/primitives/AudioFile.h"

#include "core/actions/Actions.h"
#include "units/Sampler/SamplerActions.h"

#include "common/logger.h"

#include <filesystem>

namespace slr {

SamplerView::SamplerView(Sampler * sampler) :
    AudioUnitView(sampler),
    _sampler(sampler),
    _fileAsset(nullptr)
{
    _name = "Sampler " + std::to_string(_uniqueId);
}

SamplerView::~SamplerView() {

}

void SamplerView::update() {
    _fileAsset = _sampler->asset();
    incrementVersion();
}


std::string SamplerView::unitType() { 
    std::string ret = "internal:";
    ret += *SamplerDescriptor._name;
    return ret;
}

nlohmann::ordered_json SamplerView::saveUnit() {
    nlohmann::ordered_json ret;
    ret = AudioUnitView::saveUnit();

    ret["Asset"] = _fileAsset->path();

    return ret;
}

void SamplerView::loadUnit(const nlohmann::json &data) {
    //load asset(need action)
    AudioUnitView::loadUnit(data);

    std::string path = data["Asset"].get<std::string>();
    if(std::filesystem::exists(path)) {
        auto act = std::make_unique<slr::Actions::LoadAsset>();
        act->data = path;
        act->targetId = this->id();
        EmitAction(std::move(act));
    } else {
        LOG_ERROR("Failed to load asset, path doesn't exist");
    }
}

}