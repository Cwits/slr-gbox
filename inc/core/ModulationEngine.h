// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "defines.h"
#include "core/primitives/MidiEvent.h"
 
#include <vector>

namespace slr {

struct AudioContext;

struct ParameterBase;

struct ModulationPattern {
    ParameterBase * _target;
    float _originalValue;
    float _min;
    float _max;
    float _step;

    uint16_t _stepsPerBar;
    uint16_t _bars;
};

struct ModulationEngine {
    ModulationEngine();
    ~ModulationEngine();

    frame_t process(const AudioContext &ctx);
    
    void prepareToPlay();
    void prepareToRecord();
    void stopPlaying();
    void stopRecording();

    private:
    std::vector<ModulationPattern*> _modulations;

};

}