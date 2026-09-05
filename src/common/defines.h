// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <cstdint>
#include "common/core_config.h"

namespace slr {

using frame_t = uint64_t;
using sample_t = SAMPLE_T;
using ID = uint32_t;

struct BarSize {
    uint8_t _numerator;
    uint8_t _denominator;
};

enum class TimelineState { Stop, Play, Pause, StartRecord, StopRecord };
enum class TrackArm {
    RecordArm = 0,
    SendToMixer = 1,
    Solo = 2,
};

//track
enum class RecordSource { Audio, Midi };

enum class DevicePort { INPUT, OUTPUT };


enum class StepDuration {
    dWhole      = 1,
    dHalf       = 2,
    d4triplet   = 3,
    d4          = 4,
    //d5        = 5 ??
    d8tirplet   = 6,
    //d7        = 7 ??
    d8          = 8,
    //d9        = 9 ??
    d16triplet  = 12,
    d16         = 16,
    d32triplet  = 24,
    d32         = 32,
    d64triplet  = 48,
    d64         = 64,
    d128triplet = 96,
    d128        = 128
};

}
