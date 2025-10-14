// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"

namespace slr {
    
struct AudioRoute {
    enum class Type { EXT, INT };
    
    Type _sourceType;
    ID _sourceId; 

    Type _targetType;
    ID _targetId;

    //key - target channel, value - source channel
    char _channelMap[32] = {(char)-1};
};

}