// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "display/utility/FileToCanvas.h"

#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioPeakFile.h"
#include "core/primitives/AudioPeaks.h"
#include "core/primitives/MidiFile.h"
#include "core/primitives/MidiEvent.h"
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
    canvasHeight -= 1;
    canvasWidth -= 1;
    // slr::TimelineView & tlsnap = slr::TimelineView::getTimelineView();
    //convert from ppqn to frames?
    // length = (length / slr::SettingsManager::getPpqn()) * tlsnap.framesPerQuater();
    const std::vector<slr::MidiEvent> & events = file->track(0).midiEvents;
    
    //must spread this acros width and height?
    struct Note {
        uint64_t ppqnStart = 0;
        uint64_t ppqnEnd = 0;
        int velocity = 0;
        int pitch = 0;
    };
    
    std::vector<Note> notes;
    notes.reserve(events.size());
    int noteMin = 127;
    int noteMax = 0;
    for(const slr::MidiEvent &ev : events) {
        if(ev.type == slr::MidiEventType::NoteOn) {
            Note n;
            n.ppqnStart = ev.offset;
            n.velocity = ev.velocity;
            n.pitch = ev.note;
            notes.push_back(n);
            noteMin = std::min(noteMin, n.pitch);
            noteMax = std::max(noteMax, n.pitch);
        } else if(ev.type == slr::MidiEventType::NoteOff) {
            notes.back().ppqnEnd = ev.offset;
        }
    }

    float heightPerEvent = canvasHeight/(noteMax-noteMin);
    int lastHeight = 0;
    float ratio = (float)canvasWidth / (float)notes.back().ppqnEnd;

    for(auto &n : notes) {
        int x = n.ppqnStart * ratio;
        int y = canvasHeight - ((n.pitch-noteMin) * heightPerEvent);
        int width = (n.ppqnEnd-n.ppqnStart) * ratio;
        int height = heightPerEvent;

        for(int xpos=x; xpos<width+x; ++xpos) {
            for(int ypos=y; ypos>y-height; --ypos) {
                lv_canvas_set_px(canvas, xpos, ypos, fillColor, LV_OPA_COVER);
            }
        }

        lastHeight += heightPerEvent;
    }

    return true;
}

}
