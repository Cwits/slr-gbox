// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/Metronome.h"

#include "core/primitives/AudioContext.h"
#include "core/utility/basicAudioManipulation.h"
#include "core/Timeline.h"
#include "core/SettingsManager.h"

#include "logger.h"

namespace slr {
    
Metronome::Metronome() : AudioUnit(AudioUnitType::Metronome) {
    _lastTickFrame = 0;
    _lastPlayedStep = -1;
    _remainedSamplesToPlay = 0;
    _soundLength = 4096;
    
    freq_high = 880.00f;
    freq_low = 440.00f;

    _sampleRate = SettingsManager::getSampleRate();
    _tau = 0.03 / _sampleRate; //30ms / samplingRate

    _buffersClear = false;
    *_mute = true;
}

Metronome::~Metronome() {

}

frame_t Metronome::process(const AudioContext &ctx, 
                    const Dependencies * const inputs, 
                    const uint32_t inputsCount) 
{
    // if(*_mute) return ctx.frames; - done from outside

    // if(ctx.playing) { //done from outside
    bool tick = false;
    frame_t delay = 0;
    frame_t samplesToPlay = 0;

    if(ctx.elapsed == 0) {
        tick = true;
        _lastTickFrame = ctx.elapsed;
    } else {
        frame_t framesPerBeat = ctx.timeline.framesPerBeat();
        
        if(ctx.elapsed + ctx.frames - _lastTickFrame >= framesPerBeat) {
            tick = true;
            delay = framesPerBeat - (ctx.elapsed-_lastTickFrame);
            _lastTickFrame = ctx.elapsed+delay;
        }
    }

    if(*_mute) return ctx.frames;

    if(tick) {
        _remainedSamplesToPlay = _soundLength;
        if(_soundLength > ctx.frames) {
            // _remainedSamplesToPlay = _soundLength - (ctx.frames-delay);
            samplesToPlay = ctx.frames-delay;
        } else {
            samplesToPlay = _soundLength;
        }

        
        // _remainedSamplesToPlay -= samplesToPlay;
        _lastPlayedStep++;
        if(_lastPlayedStep >= ctx.timeline.getBarSize()._numerator)
            _lastPlayedStep = 0;
    } else {
        if(_remainedSamplesToPlay > 0) {
            if(_remainedSamplesToPlay > ctx.frames)
                samplesToPlay = ctx.frames;
            else 
                samplesToPlay = _remainedSamplesToPlay;
        }

        // _remainedSamplesToPlay -= samplesToPlay;
    }

    if(samplesToPlay > 0) {    
        // LOG_INFO("Current sample %d samplesToPlay %d remainedSamples %d step %d", 
        //                 ctx.elapsed, 
        //                 samplesToPlay, 
        //                 _remainedSamplesToPlay,
        //                 _lastPlayedStep);
        clearAudioBuffers((*_outputs)[0], (*_outputs)[1], ctx.frames);

        float freq = (_lastPlayedStep == 0 ? freq_high : freq_low);
        for(frame_t s=delay; s<samplesToPlay; ++s) {
            frame_t delta = s + (_soundLength - _remainedSamplesToPlay);

            // sample_t amp = std::exp(-(float)delta / _tau) * std::sin(2.0f*M_PI * freq * (delta/44100));
            sample_t amp = 0.7*std::sin(2.0f*M_PI * freq * ((float)delta/(float)_sampleRate));
            //TODO: add simple decay
            (*_outputs)[0][s] = amp;
            (*_outputs)[1][s] = amp;
        }

        
        _remainedSamplesToPlay -= samplesToPlay;
        _buffersClear = false;
    } else {
        if(!_buffersClear) {
            clearAudioBuffers((*_outputs)[0], (*_outputs)[1], ctx.frames);
            _buffersClear = true;
        }
    }

    sumAudioBuffers((*_outputs)[0], (*ctx.mainOutputs)[0], ctx.frames);
    sumAudioBuffers((*_outputs)[1], (*ctx.mainOutputs)[1], ctx.frames);

    return ctx.frames;
}

void Metronome::prepareToPlay() {
    _lastTickFrame = 0;
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