// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/AudioRoute.h"
#include "core/primitives/AudioBuffer.h"
#include "core/primitives/MidiBuffer.h"

namespace slr {

class AudioUnit;
class Project;

/* 
    TODO: as for Audio and Midi Dependencies external\internal and id's should be resolved already to prevent 
            extra branching in rt thread

        so for audio dep it must be
        struct AudioDeps{
            const AudioBuffer *b;
            int8_t channelMap[32];
        }; 

        and for midi:
        struct MidiDeps {
            const MidiBuffer * buf;
            uint8_t channelFrom;
            uint8_t channelTo;
        };
*/

struct AudioDependencie {
    bool external; //need to get rid of this
    uint16_t extId; //and this, so

    const AudioBuffer * buffer;
    //key - target channel, value - source channel
    int8_t channelMap[32] = {-1};
};

struct MidiDependencie {
    bool external;
    uint16_t extId;

    const MidiBuffer *buf;
    uint8_t channelFrom;
    uint8_t channelTo;
};

struct Dependencies {
    AudioDependencie * audio; //should be const AudioDependencies
    uint32_t audioDepsCnt;

    MidiDependencie * midi; //should be const MidiDependencies
    uint32_t midiDepsCnt;
};

struct RenderPlan {
    //when Solo is pressed - form new, separate, plan
    //and use it, but don't get rid of full?
    struct Node {
        const AudioUnit * target;
        Dependencies deps;
        // uint32_t depsCount;
    };

    const Node * nodes;
    uint32_t nodesCount;

    Dependencies outputDeps;

    // struct SubGraphSlice {
    //     uint32_t offset;
    //     uint32_t count;
    // };

    // SubGraphSlice * slices;
    // uint32_t slicesCount;

    /* Step Sequencer Stuff
    uint32_t sequenceCount;
    const Sequence * const sequences;
    
    std::atomic<bool> syncSequences mutable; - what is this?
    */

    /* Mod Engine stuff
    uint32_t modPatternCount;
    const ModPattern * const pattern;
    */
};

RenderPlan * buildPlan(Project *prj);
void destroyPlan(const RenderPlan * plan);

inline void clearChannelMap(int8_t * map) {
    for(int i=0; i<32; ++i) {
        map[i] = -1;
    }
}

}