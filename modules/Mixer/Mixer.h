// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/AudioUnit.h"

namespace slr {

class AudioContext;

class Mixer : public AudioUnit {
    public:
    Mixer(const ClipContainer *initContainer);
    ~Mixer();

    frame_t process(const AudioContext &ctx,  const Dependencies &inputs) override;
    void prepareToPlay() override;
    void prepareToRecord() override;
    void stopPlaying() override;
    void stopRecording() override;
    
    private:
};

}