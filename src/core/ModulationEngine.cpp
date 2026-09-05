// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/ModulationEngine.h"

#include "core/utility/AudioContext.h"

#include <algorithm>

namespace slr {

ID patternIdCounter = 1;

ModulationPattern::ModulationPattern(const ID uniqueId, double sampleRate) : 
    _uniqueId(uniqueId),
    _sampleRate(sampleRate) 
{
    _targets.clearArray();
}

frame_t ModulationPattern::process(const AudioContext &ctx) const {
    //fill the _data[frame] with value(frame)
    frame_t nudge = 0;

    if(ctx.freewheeling) {
        nudge = ctx.totalFrames % static_cast<frame_t>(_sampleRate);
    } else {
        if(!ctx.playing) return ctx.frames;

        nudge = ctx.elapsed % static_cast<frame_t>(_sampleRate);
    }

    for(frame_t f=nudge; f<nudge+ctx.frames; ++f) {
        _data[f-nudge] = value(nudge+f);
    }

    return ctx.frames;
}

float ModulationPattern::value(frame_t frame) const {
    float res = 0.0;
        
    switch(_shape) {
        case(ModulationShape::sin): res = sMath::sineWave(_amplitude, static_cast<float>(frame), _sampleRate, _lfoRate, _phase); break;
        case(ModulationShape::saw): res = sMath::sawtoothWave(_amplitude, static_cast<float>(frame), _sampleRate, _lfoRate, _phase); break;
        case(ModulationShape::square): res = sMath::squareWave(_amplitude, static_cast<float>(frame), _sampleRate, _lfoRate, _phase); break;
        case(ModulationShape::tri): res = sMath::squareWave(_amplitude, static_cast<float>(frame), _sampleRate, _lfoRate, _phase); break;
        case(ModulationShape::rnd): res = 0.0; /* res = data[( ( (frame/_sampleRate or steps?) * _lfoRate) + _phase) % steps] */ break;
    }
        

    // if(_interpolate) return sMath::lerp(res, -1.f, 1.f, _valueMin, _valueMax);
    // else return res;
    return sMath::lerp(res, -1.f, 1.f, _valueMin, _valueMax);
    // LOG_INFO("frame: %lu, value: %.6f", frame, res);
    // return res;
}


void ModulationPattern::prepareToPlay() {

}

void ModulationPattern::prepareToRecord() {

}

void ModulationPattern::stopPlaying() {

}

void ModulationPattern::stopRecording() {

}

ModulationEngine::ModulationEngine() {
    _playable.reserve(16);
}

ModulationEngine::~ModulationEngine() {

}

ModulationPattern * ModulationEngine::createNewModulationPattern(const frame_t blockSize, const frame_t sampleRate) {
    if(_playable.size() >= 16) return nullptr;
    
    _modulations.push_back(std::make_unique<ModulationPattern>(patternIdCounter, sampleRate));
    patternIdCounter++;

    _dataStorage.push_back(std::unique_ptr<float>(new float[blockSize]));
    _modulations.back()->_data = _dataStorage.back().get();

    return _modulations.back().get();
}

ModulationPattern * ModulationEngine::findPatternById(ID id) {
    if(id == 0) return nullptr;

    auto found = std::find_if(
        _modulations.begin(),
        _modulations.end(),
        [id](const std::unique_ptr<ModulationPattern> & ptrn) {
            return ptrn->_uniqueId == id;
        }
    ); 

    if(found == _modulations.end()) return nullptr;

    return (*found).get();
}



}