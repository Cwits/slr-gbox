// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/AudioUnit.h"

namespace slr {

class AudioContext;
class Mixer : public AudioUnit {
    public:
    Mixer();
    ~Mixer();

    frame_t process(const AudioContext &ctx,  const Dependencies * const inputs, const uint32_t inputsCount) override;
    void prepareToPlay() override;
    void prepareToRecord() override;
    void stopPlaying() override;
    void stopRecording() override;
    
    private:
};

}