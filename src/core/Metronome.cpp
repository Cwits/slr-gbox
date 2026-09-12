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
    if(isMuted(ctx)) return ctx.blockSize;
    
    /*
        надо как то высчитывать из elapsed какой нынче шаг, потом брать % tl->barSize()._denominator 
        и... проверять какое состояние
        если текущий шаг != последний сыгранный то начать играть -> меняешь стейт на играет
        если текущий шаг == последний сыгранный -> значит либо ещё играем либо сыграли
            если играем то считаем и играем.
            если сыграли -> if(!_buffersClear) clearBuffers();
        
        нооо проблема в том что надо смотреть может ли быть шаг в текущем кадре т.к. он не всегда совпадает с ctx.elapsed
    */

    int currentStep = (ctx.elapsed + ctx.blockSize) / ctx.timeline.framesPerBeat();
    
    frame_t samplesToPlay = 0;
    frame_t delay = 0;
    if(currentStep == _lastPlayedStep) {
        if(_remainedSamplesToPlay == 0 && _buffersClear) return ctx.blockSize;

        if(_remainedSamplesToPlay >= ctx.blockSize) {
            samplesToPlay = ctx.blockSize;
        } else {
            //less than
            samplesToPlay = ctx.blockSize - _remainedSamplesToPlay;
        }

    } else if(currentStep != _lastPlayedStep) {
        _lastPlayedStep = currentStep % ctx.timeline.getBarSize()._numerator;
        _remainedSamplesToPlay = _soundLength;
        
        frame_t expectedPosition = _lastPlayedStep * ctx.timeline.framesPerBeat();
        frame_t elapsed = ctx.elapsed % ctx.timeline.framesPerBar();
        delay = (expectedPosition > ctx.elapsed) ? expectedPosition - elapsed : ctx.timeline.framesPerBar() - elapsed;
        if(expectedPosition > ctx.elapsed) expectedPosition - elapsed;
        else if(expectedPosition < ctx.elapsed) ctx.timeline.framesPerBar() - elapsed;
        
        if(delay != 0) samplesToPlay = ctx.blockSize - delay;
        else samplesToPlay = ctx.blockSize;
    }

    if(samplesToPlay > 0 && _remainedSamplesToPlay > 0) {
        _buffersClear = false;
        //play some thing
        
        clearAudioBuffers((*_outputs)[0], (*_outputs)[1], ctx.blockSize);
            
        if(delay > ctx.blockSize) {
            int i=0; 
            i+=66;
            i-= 15;
        }
        float freq = (_lastPlayedStep == 0 ? freq_high : freq_low); 
        // for(frame_t s=delay; s<samplesToPlay; ++s) {
        //     frame_t delta = s + (_soundLength - _remainedSamplesToPlay);

        //     // sample_t amp = std::exp(-(float)delta / _tau) * std::sin(2.0f*M_PI * freq * (delta/44100));
        //     sample_t amp = 0.7*sMath::sin(sMath::TWOPIF * freq * ((float)delta/(float)_sampleRate));
        //     //TODO: add simple decay
        //     (*_outputs)[0][s] = amp;
        //     (*_outputs)[1][s] = amp;
        // }

        _remainedSamplesToPlay -= samplesToPlay;
    } else {
        if(!_buffersClear) {
            clearAudioBuffers((*_outputs)[0], (*_outputs)[1], ctx.blockSize);
            _buffersClear = true;
        }
    }

    return ctx.blockSize;
}
/*
frame_t Metronome::process(const AudioContext &ctx, const Dependencies &inputs) const {
    bool tick = false;
    frame_t delay = 0;
    frame_t samplesToPlay = 0;

    if(ctx.elapsed == 0) {
        tick = true;
        _lastTickFrame = ctx.elapsed;
    } else {
        frame_t framesPerBeat = ctx.timeline.framesPerBeat();
        
        if(ctx.elapsed + ctx.blockSize - _lastTickFrame >= framesPerBeat) {
            tick = true;
            delay = framesPerBeat - (ctx.elapsed-_lastTickFrame);
            _lastTickFrame = ctx.elapsed+delay;
        }
    }

    if(isMuted(ctx)) return ctx.blockSize;

    if(tick) {
        _remainedSamplesToPlay = _soundLength;
        if(_soundLength > ctx.blockSize) {
            // _remainedSamplesToPlay = _soundLength - (ctx.blockSize-delay);
            samplesToPlay = ctx.blockSize-delay;
        } else {
            samplesToPlay = _soundLength;
        }

        
        // _remainedSamplesToPlay -= samplesToPlay;
        _lastPlayedStep++; //lastPlayedStep counted wrong when looping
        if(_lastPlayedStep >= ctx.timeline.getBarSize()._numerator)
            _lastPlayedStep = 0;
    } else {
        if(_remainedSamplesToPlay > 0) {
            if(_remainedSamplesToPlay > ctx.blockSize)
                samplesToPlay = ctx.blockSize;
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
        _buffersClear = false;
    } else {
        if(!_buffersClear) {
            clearAudioBuffers((*_outputs)[0], (*_outputs)[1], ctx.blockSize);
            _buffersClear = true;
        }
    }

    sumAudioBuffers((*_outputs)[0], (*ctx.mainOutputs)[0], ctx.blockSize);
    sumAudioBuffers((*_outputs)[1], (*ctx.mainOutputs)[1], ctx.blockSize);

    return ctx.blockSize;
}
*/
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