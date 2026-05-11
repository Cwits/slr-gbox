// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "modules/Sampler/SamplerView.h"

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
}

}