// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"
#include "core/primitives/AudioBuffer.h"
#include <vector>

namespace slr {

namespace AudioBufferManager {
    bool init(int blockSize, int channels);
    bool shutdown();

    AudioBuffer * acquireRegular();
    void releaseRegular(AudioBuffer * b);
    bool expandRegularPool();
    const int freeRegularCount();
    const int totalRegularCount();
    
    RT_FUNC AudioBuffer * acquireRecord();
    RT_FUNC void releaseRecord(AudioBuffer * b);
    bool expandRecordPool();
    const int totalRecordCount();

    const frame_t recordBufferSize();
}

}
