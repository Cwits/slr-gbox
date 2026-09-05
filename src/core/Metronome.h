// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/AudioUnit.h"

namespace slr {

struct Metronome : public AudioUnit {
    Metronome();
    ~Metronome();

    frame_t process(const AudioContext &ctx, const Dependencies &inputs) const override;

    void prepareToPlay() override;
    void prepareToRecord() override;
    void stopPlaying() override;
    void stopRecording() override;

    frame_t latency() override;

    private:
    mutable frame_t _lastTickFrame;
    mutable frame_t _remainedSamplesToPlay;

    float _tau;
    frame_t _sampleRate;

    mutable int _lastPlayedStep;

    frame_t _soundLength;
    float freq_high;
    float freq_low;
};

}