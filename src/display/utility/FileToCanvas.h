// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lvgl.h"
#include "common/defines.h"

namespace slr {
    class AudioFile;
    class AudioPeaks;
    class MidiFile;
    class MidiTrack;
}

namespace UIHelpers {


void audioFileToCanvas( 
    const slr::AudioPeaks * const file,
    slr::frame_t fileStart,
    slr::frame_t length,
    lv_obj_t * canvas,
    int canvasHeight,
    int canvasWidth,
    lv_color_t peakColor,
    lv_color_t fillColor);


void midiFileToCanvas( 
    const slr::MidiTrack * const file,
    slr::frame_t fileStart,
    slr::frame_t length,
    int noteFrom,
    int noteTo,
    lv_obj_t * canvas,
    int canvasHeight,
    int canvasWidth,
    lv_color_t peakColor,
    lv_color_t fillColor);

}
