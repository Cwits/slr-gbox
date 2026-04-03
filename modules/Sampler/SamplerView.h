// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once 

#include "modules/Sampler/Sampler.h"

#include "snapshots/AudioUnitView.h"
#include "defines.h"

#include <string>

namespace slr {

struct SamplerView : public AudioUnitView {
    public:
    SamplerView(Sampler * sampler);
    ~SamplerView();
    
    void update() override;

    const AudioFile * asset() const { return _fileAsset; }
    private:
    const Sampler * _sampler;
    const AudioFile * _fileAsset; 

    uint8_t _maxVoices;
};


}