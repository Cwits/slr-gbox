// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"
#include <cstdint>

namespace slr {
    
struct MidiRoute {
    enum class Type { EXT, INT };
    
    Type _sourceType;
    ID _sourceId; 
    int8_t _sourceChannel = 0; //0 - omni

    Type _targetType;
    ID _targetId;
    int8_t _targetChannel = 0;
};


}
