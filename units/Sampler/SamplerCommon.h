// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "common/defines.h"

constexpr int DEFAULT_ROOT_PITCH = 36;
constexpr int MAX_VOICES = 32;
constexpr int DEFAULT_VOICES = 8;

enum class SamplerMode {
    Classic,
    OneShot,
    Slicer
};


struct SlicerSettings {
    enum class Mode {
        Transient,
        Beat,
        Manual
    };

    Mode _mode;
    float sensetivity; //for transient, range [0.0, 1.0]
    slr::frame_t minimalSpacing; //for transient, in samples

    int beatGrid;
};

struct Slice {
    bool active;
    slr::frame_t sliceStart;
    slr::frame_t length;
};