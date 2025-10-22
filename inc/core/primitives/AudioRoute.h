// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"
#include <cstdint>

namespace slr {
    
struct AudioRoute {
    enum class Type { EXT, INT };
    
    Type _sourceType;
    ID _sourceId; 

    Type _targetType;
    ID _targetId;

    //key - target channel, value - source channel
    int8_t _channelMap[32] = {-1};

    static void clearChannels(AudioRoute &r) {
        for(int i=0; i<32; ++i) {
            r._channelMap[i] = -1;
        }
    }
};


}