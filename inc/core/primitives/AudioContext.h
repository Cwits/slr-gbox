// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "defines.h"
#include "core/primitives/AudioBuffer.h"
#include "core/FlatEvents.h"
#include "core/primitives/SPSCQueue.h"

namespace slr { 
// class AudioBuffer;
class Timeline;

struct AudioContext {
    public:
    AudioContext(const bool playing,
                const bool recording,
                const frame_t frames,
                const frame_t elapsed, 
                const frame_t totalFrames,
                const AudioBuffer * in, 
                AudioBuffer * const out, 
                const Timeline & tl,
                SPSCQueue<FlatEvents::FlatResponse, 256> & outControl) 
                :
                playing(playing),
                recording(recording),
                frames(frames),
                elapsed(elapsed),
                totalFrames(totalFrames),
                mainInputs(in), 
                mainOutputs(out),
                timeline(tl),
                outputControl(outControl) {}
    
    const bool playing;
    const bool recording;
    
    const frame_t frames; //ammount of frames to process == block size
    const frame_t elapsed; //if playing than diff, else = 0
    const frame_t totalFrames;//total ammount frames passed since audio driver started

    const AudioBuffer * mainInputs;
    AudioBuffer * const mainOutputs;

    const Timeline & timeline;
    SPSCQueue<FlatEvents::FlatResponse, 256> & outputControl;
    
    //array::SPSCQueue<MidiEvents> _midiInputs; - hw
    //array::SPSCQueue<MidiOutputs> _midiOutputs; - hw
};

}