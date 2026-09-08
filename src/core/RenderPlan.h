// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/AudioRoute.h"
#include "core/primitives/AudioBuffer.h"
#include "core/primitives/MidiBuffer.h"
#include "common/defines.h"

#include <memory>

namespace slr {

struct AudioUnit;
struct Project;
struct ModulationPattern;
struct Sequence;
struct Timeline;
struct Metronome;
struct ControlContext;

/* 
    TODO: as for Audio and Midi Dependencies external\internal and id's should be resolved already to prevent 
            extra branching in rt thread - but that's impossible, because some audio drivers might not have constant buffer pointers...

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
    bool external; 
    uint16_t extId; 

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
    const AudioDependencie * audio; //should be const AudioDependencies
    uint32_t audioDepsCnt;

    const MidiDependencie * midi; //should be const MidiDependencies
    uint32_t midiDepsCnt;
};

struct RenderPlan {
    const Timeline * timeline;
    const Metronome * metro;

    struct Node {
        const AudioUnit * target;
        Dependencies deps;
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

    const ModulationPattern * modulationPatterns = nullptr;
    uint32_t modPatternCount = 0;
    
    const Sequence* sequences = nullptr;
    uint32_t sequenceCount = 0;
};

namespace PlanBuilder {

struct DepHolder {
    std::vector<AudioDependencie> audioDep;
    std::vector<MidiDependencie> midiDep;
};

struct PlanHolder {
    std::unique_ptr<RenderPlan> plan;
    std::vector<RenderPlan::Node> nodes;
    std::vector<DepHolder> nodeDepHolder;
    DepHolder outDeps;
    std::vector<ModulationPattern*> modPatterns;
    std::vector<Sequence*> sequences; 
};

enum class PlanRebuild {
    All = 0xFFFF,
    Units = 1,
    Modulations = 2,
    Sequences = 4
};

bool clearPlan(RenderPlan * plan);
bool clearHolder(PlanHolder *holder);
bool buildUnits(ControlContext &ctx, PlanHolder *holder);
bool copyUnits(PlanHolder *dst, const PlanHolder *src);

bool buildModulations(ControlContext &ctx, PlanHolder *holder);
bool copyModulations(PlanHolder *dst, const PlanHolder *src);

bool buildSequences(ControlContext &ctx, PlanHolder *holder);
bool copySequences(PlanHolder *dst, const PlanHolder *src);
}

// const RenderPlan * buildPlan(uint16_t bitmask, Project *prj);
// void destroyPlan(const RenderPlan * plan);

inline void clearChannelMap(int8_t * map) {
    for(int i=0; i<32; ++i) {
        map[i] = -1;
    }
}

}