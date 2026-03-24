// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/AudioUnit.h"
#include "core/primitives/AudioBuffer.h"
#include "core/FlatEvents.h"
#include "defines.h"

namespace slr {

class SimpleOsc : public AudioUnit {
    public:
    SimpleOsc(const ClipContainer *initContainer);
    ~SimpleOsc();

    RT_FUNC frame_t process(const AudioContext &ctx, const Dependencies &inputs) override;
    
    RT_FUNC void prepareToPlay() override;
    RT_FUNC void prepareToRecord() override;
    RT_FUNC void stopPlaying() override;
    RT_FUNC void stopRecording() override;

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

    voice _voices[16];
    int _activeVoices = 0;

};

}