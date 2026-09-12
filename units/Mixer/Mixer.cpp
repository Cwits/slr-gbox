// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/Mixer/Mixer.h"
#include "core/utility/AudioContext.h"
#include "core/utility/basicAudioManipulation.h"
#include "core/RenderPlan.h"

namespace slr {

Mixer::Mixer(const ClipContainer *initContainer, const ID forcedId) : AudioUnit(initContainer, forcedId) {

}

Mixer::~Mixer() {

}

frame_t Mixer::process(const AudioContext &ctx,  const Dependencies &inputs) const {
    if(isMuted(ctx)) return ctx.blockSize;

    clearAudioBuffer((*_outputs)[0], ctx.blockSize);
    clearAudioBuffer((*_outputs)[1], ctx.blockSize);

    if(ctx.playing) {
        playbackFiles(ctx, _outputs, _midiInput);
    }

    for(uint32_t i=0; i<inputs.audioDepsCnt; ++i) {    
        const AudioDependencie &ext = inputs.audio[i];
        const AudioBuffer * source = ext.external ? ctx.mainInputs : ext.buffer;

        //mix to preFX
        for(int ch=0; ch<32; ++ch) {
            if(ext.channelMap[ch] == -1) continue;

            for(frame_t f=0; f<ctx.blockSize; ++f) {
                (*_outputs)[ch][f] += (*source)[ext.channelMap[ch]][f];
            }
        }
    }

    return ctx.blockSize;
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