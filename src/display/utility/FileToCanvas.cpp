// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "display/utility/FileToCanvas.h"

#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioPeakFile.h"
#include "core/primitives/AudioPeaks.h"
#include "core/primitives/MidiFile.h"
#include "core/primitives/MidiEvent.h"
#include "core/primitives/MidiPeaks.h"
#include "core/SettingsManager.h"

#include "snapshots/TimelineView.h"

#include "common/Math.h"
#include "common/logger.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace UIHelpers {

void audioFileToCanvas(
    const slr::AudioPeaks * const peakFile,
    slr::frame_t fileStart,
    slr::frame_t length,
    lv_obj_t * canvas,
    int canvasHeight,
    int canvasWidth,
    lv_color_t peakColor,
    lv_color_t fillColor)
{
     float pickratio = (float)length / (float)canvasWidth; 
    // slr::LODLevels nearestLvl = slr::AudioPeaks::pickLevel(pickratio);

    const slr::PeakData0 * buffer = peakFile->peaks0();

    int channels = buffer->channels();
    float diff = length - fileStart;
    float ratio = (float)diff / (float)canvasWidth;
    // float ratio = (float)buffer->size()/(float)canvasWidth;

    int heightPerChannel = (canvasHeight/channels);
    int lerpMin = 0;
    int lerpMax = heightPerChannel;

    if(ratio < 1.0f) {
        //less than sample per pixel
    } else {
        for(int ch=0; ch<channels; ++ch) {
            for(int x=0; x<canvasWidth; ++x) {
                int offset = (ratio*x) + fileStart;

                uint8_t min = 255;
                uint8_t max = 0;
                for(int off=offset; off<offset+ratio; ++off) {
                    min = Math::min(min, (*buffer)[ch][off]);
                    max = Math::max(max, (*buffer)[ch][off]);
                }

                //TODO: this + after map is odd... need to fix it
                uint16_t dmin = Math::lerp<uint16_t>(min, 0, 255, lerpMin, lerpMax);
                uint16_t dmax = Math::lerp<uint16_t>(max, 0, 255, lerpMin, lerpMax);

                for(int y=dmin; y<dmax; ++y) {
                    lv_color_t & color = (y == dmin) ? peakColor : ( (y == dmax-1) ? peakColor : fillColor );
                    lv_canvas_set_px(canvas, x, y, color, LV_OPA_COVER);
                }
            }
            lerpMin += heightPerChannel;
            lerpMax += heightPerChannel;
        }
    }
}

struct Note {
    slr::frame_t ppqnStart = 0;
    slr::frame_t ppqnEnd = 0;
    int velocity = 0;
    int note = 0;
};

/*
    leave noteFrom and noteTo == -1 if want to auto pick range
    fileStart and length must be in frames
*/
void midiFileToCanvas(
    const slr::MidiPeaks * const peaks,
    slr::frame_t fileStart,
    slr::frame_t length,
    int noteFrom,
    int noteTo,
    lv_obj_t * canvas,
    int canvasHeight,
    int canvasWidth,
    lv_color_t peakColor,
    lv_color_t fillColor)
{
    if (!peaks || !canvas ||
        canvasWidth <= 0 || canvasHeight <= 0 ||
        length <= 0) {
        return;
    }

    slr::TimelineView &tl = slr::TimelineView::getTimelineView();

    const auto framesPerQuarter = tl.framesPerQuater();
    const auto ppqn = tl.ppqn();

    if(framesPerQuarter <= 0 || ppqn <= 0) return;

    const slr::frame_t start = (fileStart / framesPerQuarter) * ppqn;
    const slr::frame_t end = ((fileStart + length) / framesPerQuarter) * ppqn;

    if(end <= start) return;

    const std::vector<slr::Note> &notes = peaks->notes();

    int noteMin = 128;
    int noteMax = 0;

    const bool pickRange = noteFrom == -1 || noteTo == -1;

    if(!pickRange) {
        noteMin = Math::clamp(noteFrom, 0, 127);
        noteMax = Math::clamp(noteTo, 0, 127);

        if(noteMin > noteMax) return;
    }

    if(pickRange) {
        for(const auto &n : notes) {
            noteMin = std::min(noteMin, n.note);
            noteMax = std::max(noteMax, n.note);
        }
    }

    if(pickRange) {
        if(noteMin > noteMax) {
            LOG_ERROR("Note min %d can't be bigger than note max %d", noteMin, noteMax);
            return;
        }
    }

    const int noteCount = noteMax - noteMin + 1;
    if(noteCount <= 0) return;

    const double pixelsPerTick = static_cast<double>(canvasWidth) / static_cast<double>(end - start);
    const double pixelsPerNote = static_cast<double>(canvasHeight) / noteCount;

    for(const auto &n : notes) {
        if(n.note < noteMin || n.note > noteMax) continue;

        const auto clippedStart = Math::max(n.ppqnStart, start);
        const auto clippedEnd = Math::min(n.ppqnEnd, end);

        if(clippedEnd <= clippedStart) continue;

        int x1 = static_cast<int>((clippedStart - start) * pixelsPerTick);
        int x2 = static_cast<int>((clippedEnd - start) * pixelsPerTick);

        x1 = Math::clamp(x1, 0, canvasWidth - 1);
        x2 = Math::clamp(x2, x1 + 1, canvasWidth);

        const int row = noteMax - n.note;
            
        int y1 = static_cast<int>(Math::floor(row * pixelsPerNote));
        int y2 = static_cast<int>(Math::ceil((row + 1) * pixelsPerNote));

        y1 = Math::clamp(y1, 0, canvasHeight - 1);
        y2 = Math::clamp(y2, y1 + 1, canvasHeight);

        const lv_color_t color = n.velocity >= 100 ? peakColor : fillColor;

        for(int x=x1; x<x2; ++x) {
            for(int y=y1; y<y2; ++y) {
                lv_canvas_set_px(canvas, x, y, color, LV_OPA_COVER);
            }
        }
    }
}


}