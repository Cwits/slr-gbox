// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>
#include <atomic>
#include "Status.h"
#include "core/primitives/AudioBuffer.h"
#include "core/primitives/AudioRoute.h"

namespace slr {

class AudioUnit;
class Project;

struct Dependencies {
    bool external;
    uint16_t extId;

    const AudioBuffer * buffer;
    
    //key - target channel, value - source channel
    int8_t channelMap[32] = {-1};
};

struct RenderPlan {
    //when Solo is pressed - form new, separate, plan
    //and use it, but don't get rid of full?
    struct Node {
        AudioUnit * target;
        Dependencies * deps;
        uint32_t depsCount;
    };

    Node * nodes;
    uint32_t nodesCount;

    Dependencies * mixerDeps;
    uint32_t mixerDepsCount;

    // struct SubGraphSlice {
    //     uint32_t offset;
    //     uint32_t count;
    // };

    // SubGraphSlice * slices;
    // uint32_t slicesCount;


    //for audio driver
    // Dependencies * driverDeps;
    // uint32_t driverDepsCount;
};

RenderPlan * buildPlan(Project * prj, const std::vector<AudioRoute> & routes);
void destroyPlan(const RenderPlan * plan);

}