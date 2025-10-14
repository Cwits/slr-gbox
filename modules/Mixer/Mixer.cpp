// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/Mixer.h"
#include "core/primitives/AudioContext.h"
#include "core/utility/basicAudioManipulation.h"
#include "core/primitives/RenderPlan.h"

namespace slr {

Mixer::Mixer() : AudioUnit(AudioUnitType::Mixer) {

}

Mixer::~Mixer() {

}

frame_t Mixer::process(const AudioContext &ctx,  const Dependencies * const inputs, const uint32_t inputsCount)  {
    if(*_mute) return ctx.frames;

    clearAudioBuffer((*_outputs)[0], ctx.frames);
    clearAudioBuffer((*_outputs)[1], ctx.frames);

    for(uint32_t i=0; i<inputsCount; ++i) {
        for(frame_t f=0; f<ctx.frames; ++f) {
            (*_outputs)[0][f] += (*inputs[i].buffer)[0][f];
            (*_outputs)[1][f] += (*inputs[i].buffer)[1][f];
        }
    }

    //this souldn't be here but in RtEngine...
    copyAudioBuffer((*_outputs)[0], (*ctx.mainOutputs)[0], ctx.frames);
    copyAudioBuffer((*_outputs)[1], (*ctx.mainOutputs)[1], ctx.frames);


    return ctx.frames;
}

void Mixer::prepareToPlay() {

}

void Mixer::prepareToRecord() {

}

void Mixer::stopPlaying() {

}

void Mixer::stopRecording() {
    
}

}