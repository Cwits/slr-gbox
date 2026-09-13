// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/Metronome.h"

#include "core/utility/AudioContext.h"
#include "core/utility/basicAudioManipulation.h"
#include "core/Timeline.h"
#include "core/SettingsManager.h"

#include "common/Math.h"
#include "common/logger.h"

namespace slr {
    
Metronome::Metronome() : AudioUnit(nullptr, 0) {
    // _lastTickFrame = 0;
    // _lastPlayedStep = -1;
    _remainedSamplesToPlay = 0;
    _soundLength = 4096;
    
    freq_high = 880.00f;
    freq_low = 440.00f;

    _sampleRate = SettingsManager::getSampleRate();
    _tau = 0.03 / _sampleRate; //30ms / samplingRate

    _buffersClear = false;
    _mute = true;
}

Metronome::~Metronome() {

}

frame_t Metronome::process(const AudioContext &ctx, const Dependencies &inputs) const {
    if(_mute) {
        return ctx.blockSize;
    }
    
    //TODO: still can be optimized, e.g. calc frame_t elapsed = ctx.elapsed % ctx.timeline.framesPerBar();
    //now and than calc everything else
    int currentStep = (ctx.elapsed + ctx.blockSize) / ctx.timeline.framesPerBeat();
    currentStep = currentStep % ctx.timeline.getBarSize()._numerator;

    frame_t samplesToPlay = 0;
    frame_t delay = 0;
    if(currentStep == _lastPlayedStep) {
        if(_remainedSamplesToPlay == 0 && _buffersClear) return ctx.blockSize;

        if(_remainedSamplesToPlay >= ctx.blockSize) {
            samplesToPlay = ctx.blockSize;
        } else if(_remainedSamplesToPlay < ctx.blockSize && _remainedSamplesToPlay > 0) {
            //less than
            samplesToPlay = ctx.blockSize - _remainedSamplesToPlay;
        }

    } else if(currentStep != _lastPlayedStep) {
        _lastPlayedStep = currentStep;
        _remainedSamplesToPlay = _soundLength;
        
        frame_t expectedPosition = _lastPlayedStep * ctx.timeline.framesPerBeat();
        frame_t elapsed = ctx.elapsed % ctx.timeline.framesPerBar();

        if(expectedPosition > elapsed) expectedPosition - elapsed;
        else if(expectedPosition < elapsed) elapsed;
        
        if(delay != 0) samplesToPlay = ctx.blockSize - delay;
        else samplesToPlay = ctx.blockSize;
    }

    if(samplesToPlay == 0 || _remainedSamplesToPlay == 0) {
        return ctx.blockSize;
    }

    _buffersClear = false;
    //play some thing
        
    clearAudioBuffers((*_outputs)[0], (*_outputs)[1], ctx.blockSize);

    float freq = (_lastPlayedStep == 0 ? freq_high : freq_low); 
    for(frame_t s=delay; s<samplesToPlay; ++s) {
        frame_t delta = s + (_soundLength - _remainedSamplesToPlay);

        // sample_t amp = std::exp(-(float)delta / _tau) * std::sin(2.0f*M_PI * freq * (delta/44100));
        sample_t amp = 0.7*sMath::sin(sMath::TWOPIF * freq * ((float)delta/(float)_sampleRate));
        //TODO: add simple decay
        (*_outputs)[0][s] = amp;
        (*_outputs)[1][s] = amp;
    }

    _remainedSamplesToPlay -= samplesToPlay;
    
    sumAudioBuffers((*_outputs)[0], (*ctx.mainOutputs)[0], ctx.blockSize);
    sumAudioBuffers((*_outputs)[1], (*ctx.mainOutputs)[1], ctx.blockSize);

    return ctx.blockSize;
}

void Metronome::prepareToPlay() {
    // _framesTillTick = 0;
    _lastPlayedStep = -1;
    _remainedSamplesToPlay = 0;
}

void Metronome::prepareToRecord() {

}

void Metronome::stopPlaying() {

}

void Metronome::stopRecording() {

}

frame_t Metronome::latency() {
    return 0;
}

}