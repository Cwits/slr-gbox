// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "common/defines.h"
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

    bool operator==(const AudioRoute &other) const {
        bool main = (_sourceType == other._sourceType &&
                _sourceId == other._sourceId &&
                _targetType == other._targetType &&
                _targetId == other._targetId);
        bool channels = true;
        for(int i=0; i<32; ++i) {
            if(_channelMap[i] != other._channelMap[i]) {
                channels = false;
                break;
            }
        }
        return main && channels;
    }

    bool operator!=(const AudioRoute &other) const { 
        return !(*this == other);
    }
};


}