// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/AudioUnit.h"
#include "core/primitives/AudioBuffer.h"
#include "common/defines.h"

namespace slr {

class SimpleOsc : public AudioUnit {
    public:
    SimpleOsc(const ClipContainer *initContainer, const ID forcedId);
    ~SimpleOsc();

    frame_t process(const AudioContext &ctx, const Dependencies &inputs) const override;
    
    void prepareToPlay() override;
    void prepareToRecord() override;
    void stopPlaying() override;
    void stopRecording() override;

    private:
    
    float _deltaTime;
    float _time;
    float _phase;

    struct voice {
        int pitch;
        int velocity;
        int framesDelay;
        float _time;
    };

    mutable voice _voices[16];
    mutable int _activeVoices = 0;

};

}