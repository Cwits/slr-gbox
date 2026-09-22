// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "display/utility/FileToCanvas.h"

#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioPeakFile.h"
#include "core/primitives/AudioPeaks.h"
#include "core/primitives/MidiFile.h"
#include "core/SettingsManager.h"

#include "snapshots/TimelineView.h"

namespace UIHelpers {

static int map(int x, int in_min, int in_max, int out_min, int out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

bool audioFileToCanvas( 
    const slr::AudioFile * const file,
    slr::frame_t fileStart,
    slr::frame_t length,
    lv_obj_t * canvas,
    int canvasHeight,
    int canvasWidth,
    lv_color_t peakColor,
    lv_color_t fillColor) 
{
    const slr::AudioPeaks * peakFile = file->peaks();

    int xsize = canvasWidth;
    float pickratio = (float)length / (float)xsize; 
    // slr::LODLevels nearestLvl = slr::AudioPeaks::pickLevel(pickratio);

    const slr::PeakData0 * buffer = peakFile->peaks0();

    int channels = buffer->channels();
    float ratio = (float)buffer->size()/(float)xsize;

    int heightPerChannel = (canvasHeight/channels);
    int midpoint = 0 + (heightPerChannel/channels);
    // int center = midpoint;//+(heightPerChannel/2);

    if(ratio < 1.0f) {
        //less than sample per pixel
    } else {
        for(int ch=0; ch<channels; ++ch) {
            for(int x=0; x<xsize; ++x) {
                int offset = ratio*x;

                uint8_t min = 255;
                uint8_t max = 0;
                for(int off=offset; off<offset+ratio; ++off) {
                    min = std::min(min, (*buffer)[ch][off]);
                    max = std::max(max, (*buffer)[ch][off]);
                }

                //TODO: this + after map is odd... need to fix it
                uint8_t dmin = map(min, 0, 255, 0, heightPerChannel) + (ch == 0 ? 0 : heightPerChannel);
                uint8_t dmax = map(max, 0, 255, 0, heightPerChannel) + (ch == 0 ? 0 : heightPerChannel);
                // if(dmax > TRACK_HEIGHT) dmax = TRACK_HEIGHT;
                for(int y=dmin; y<dmax; ++y) {
                    lv_color_t & color = (y == dmin) ? peakColor : ( (y == dmax) ? peakColor : fillColor );
                    lv_canvas_set_px(canvas, x, y, color, LV_OPA_COVER);
                }
            }
                    
            // midpoint += heightPerChannel;
        }
    }

    return true;
}


bool midiFileToCanvas( 
    const slr::MidiFile * const file,
    slr::frame_t fileStart,
    slr::frame_t length,
    lv_obj_t * canvas,
    int canvasHeight,
    int canvasWidth,
    lv_color_t peakColor,
    lv_color_t fillColor) 
{
    // slr::TimelineView & tlsnap = slr::TimelineView::getTimelineView();
    //convert from ppqn to frames?
    // length = (length / slr::SettingsManager::getPpqn()) * tlsnap.framesPerQuater();



    return true;
}

}
