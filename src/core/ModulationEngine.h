// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "common/defines.h"
#include "common/ModEngineCommon.h"
#include "core/primitives/MidiEvent.h"
 
#include "common/logger.h"
#include "common/Math.h"

#include <vector>
#include <cmath>
#include <memory>

namespace slr {

struct AudioContext;

struct ParameterBase;

struct ModulationPattern {
    ModulationPattern(const ID uniqueId, double sampleRate);
    ~ModulationPattern() = default;
    
    frame_t process(const AudioContext &ctx) const;
    
    void prepareToPlay();
    void prepareToRecord();
    void stopPlaying();
    void stopRecording();

    float value(frame_t frame) const;

    const ID _uniqueId;

    ModulationShape _shape = ModulationShape::sin;
    
    float _sampleRate;
    float _amplitude = 1.0f;
    float _phase = 0.0f;

    float _lfoRate = 1.0f;
    bool _rateMode = true; //when true - rate is in hertz(range 0 to ... idk... 1000?), when false - rate represents beat e.g. 1/16. 1/4, etc.

    // bool _interpolate = false; //put to true if any of valueMin or valueMax ever changed
    float _valueMin = -1.0f; //interpolate between normalized res from value and this values
    float _valueMax = 1.0f;
    
    ModTargetArray _targets;

    /* custom shape - screw this? don't need. Better add random with seed... orr??? later
        int steps;
        float * data; 

        std::array<float, steps>
    */

    mutable float * _data;
};

struct ModulationEngine {
    ModulationEngine();
    ~ModulationEngine();

    ModulationPattern * createNewModulationPattern(const frame_t blockSize, const frame_t sampleRate);
    ModulationPattern * findPatternById(ID id);

    std::vector<ModulationPattern*> allPatterns() const;
    
    //TODO: hack, get rid of this
    std::vector<std::unique_ptr<ModulationPattern>> & notToUsePatterns() { return _modulations; }
    private:
    std::vector<std::unique_ptr<ModulationPattern>> _modulations;
    std::vector<std::unique_ptr<float>> _dataStorage;
};

}