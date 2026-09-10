// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "core/primitives/AudioBuffer.h"
#include "core/primitives/MidiBuffer.h"
#include "common/defines.h"

#include <unordered_map>
#include <vector>

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
                std::vector<RtMidiBuffer> *midiInputs)
                :
                playing(playing),
                recording(recording),
                frames(frames),
                elapsed(elapsed),
                totalFrames(totalFrames),
                mainInputs(in), 
                mainOutputs(out),
                timeline(tl),
                // outputControl(outControl),
                midiInputs(midiInputs)
                 {}
    
    const bool playing;
    const bool recording;
    
    const frame_t frames; //ammount of frames to process == block size
    const frame_t elapsed; //if playing than diff, else = 0
    const frame_t totalFrames;//total ammount frames passed since audio driver started

    const AudioBuffer * mainInputs;
    AudioBuffer * const mainOutputs;

    const Timeline & timeline;
    
    const std::vector<RtMidiBuffer> * const midiInputs;
    //array::SPSCQueue<MidiOutputs> _midiOutputs; - hw
};

inline const MidiBuffer * getMidiBuffer(const AudioContext &ctx, const ID &id) {
    for(const RtMidiBuffer &p : *ctx.midiInputs) {
        if(p.id == id) {
            return p.buffer;
        } 
    }

    //theoretically unreachable
    assert(false);
    return nullptr;
}

}
