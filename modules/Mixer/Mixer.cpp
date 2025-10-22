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
        const Dependencies &ext = inputs[i];
        const AudioBuffer * source = ext.external ? ctx.mainInputs : ext.buffer;

        //mix to preFX
        for(int ch=0; ch<32; ++ch) {
            if(ext.channelMap[ch] == -1) continue;

            for(frame_t f=0; f<ctx.frames; ++f) {
                (*_outputs)[ch][f] += (*source)[ext.channelMap[ch]][f];
            }
        }
    }

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