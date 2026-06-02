// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "defines.h"
#include "core/primitives/MidiEvent.h"

#include <vector>

namespace slr {

struct AudioUnit;
struct AudioContext;

struct Sequence {
    
    AudioUnit * _target;

    uint16_t _stepsPerBar;
    uint16_t _bars;
    std::vector<MidiEvent> _events;
};

struct StepSequencerEngine {
    StepSequencerEngine();
    ~StepSequencerEngine();

    frame_t process(const AudioContext &ctx);
    
    void prepareToPlay();
    void prepareToRecord();
    void stopPlaying();
    void stopRecording();

    private:
    std::vector<Sequence*> _sequences;
};


}