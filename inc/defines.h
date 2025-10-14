// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <cstdint>

namespace slr {

//just to make easier to understand
#define RT_FUNC

using frame_t = uint64_t;

#define SAMPLE_T float
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

}
