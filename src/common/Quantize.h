// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "common/defines.h"
#include "common/Math.h"

namespace slr {

namespace Quantizer {

frame_t simpleQuantize(frame_t frame, frame_t framesPerBar, int grid) {
    return frame;
    /*
    if(grid == 0) return frame;

    double framesPerStep = framesPerBar / static_cast<double>(grid);
    double error = framesPerStep - Math::floor(framesPerStep);

    frame_t threshold = framesPerStep / 2;
    frame_t fullSteps = frame / static_cast<frame_t>(framesPerStep);
    frame_t ret = 0;

    if(frame < threshold) { 
        frame_t tmp = fullSteps * static_cast<frame_t>(framesPerStep);
        ret = tmp;
    } else {
        frame_t tmp = (fullSteps+1) * static_cast<frame_t>(framesPerStep);
        ret = tmp;
    }

    if(error != 0.0f) {
        frame_t addFrames = static_cast<double>(fullSteps+1) * error;
        ret += addFrames;
    }

    return ret;
    */
}

} //namespace quantizer

} //namespace slr