// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once 

#include "units/Sampler/Sampler.h"
#include "units/Sampler/SamplerCommon.h"

#include "snapshots/AudioUnitView.h"
#include "common/defines.h"

#include <string>

namespace slr {

struct SamplerView : public AudioUnitView {
    public:
    SamplerView(Sampler * sampler);
    ~SamplerView();
    
    void update() override;

    const std::array<slr::Voice, MAX_VOICES> & voices() const { return _sampler->voices(); }

    const AudioFile * asset() const { return _fileAsset; }

    SamplerMode mode() const { return _mode; }
    void mode(SamplerMode m) { _mode = m; incrementVersion(); }

    std::string unitType() override;
    
    nlohmann::ordered_json saveUnit() override;
    void loadUnit(const nlohmann::json &data) override;

    SamplerMode _mode;
    // bool _adsrEnabled;

    ParameterIntView *_rootPitch;
    ParameterIntView *_maxVoices;
    ParameterFrameView *_sampleStart;
    ParameterFrameView *_sampleEnd;

    private:
    const Sampler * _sampler;
    const AudioFile * _fileAsset; 

};


}