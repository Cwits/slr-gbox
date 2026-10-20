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

#include "common/Math.h"

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
    float ratio = (float)buffer->size()/(float)canvasWidth;

    int heightPerChannel = (canvasHeight/channels);
    int lerpMin = 0;
    int lerpMax = heightPerChannel;

    if(ratio < 1.0f) {
        //less than sample per pixel
    } else {
        for(int ch=0; ch<channels; ++ch) {
            for(int x=0; x<canvasWidth; ++x) {
                int offset = ratio*x;

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
    int pitch = 0;
};

/*
    leave noteFrom and noteTo == 0 if want to auto pick range
    fileStart and length must be in frames
*/
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
    lv_color_t fillColor) 
{
    const std::vector<slr::MidiEvent> & events = file->midiEvents;
    std::vector<Note> notes;

    int noteMin = 127;
    int noteMax = 0;
    
    bool pickRange = true;
    if(noteFrom != 0 && noteTo != 0) {
        //range given manually
        pickRange = false;
        noteMin = noteFrom;
        noteMax = noteTo;
    }
    
    slr::TimelineView &tl = slr::TimelineView::getTimelineView();

    slr::frame_t start = (fileStart / tl.framesPerQuater()) * tl.ppqn();
    slr::frame_t end = (length / tl.framesPerQuater()) * tl.ppqn();

    for(auto &ev : events) {
        //skip events that not in range (but may loose some of events unfortunately)
        if(ev.offset < start) continue;
        if(ev.offset > end) continue;
        
        if(ev.type == slr::MidiEventType::NoteOn) {
            Note n;
            n.ppqnStart = ev.offset;
            n.velocity = ev.velocity;
            n.pitch = ev.note;
            notes.push_back(n);
            
            if(pickRange) {
                noteMin = std::min(noteMin, n.pitch);
                noteMax = std::max(noteMax, n.pitch);
            }
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

        if(y == canvasHeight) y -= 1;

        for(int xpos=x; xpos<width+x; ++xpos) {
            for(int ypos=y; ypos>y-height; --ypos) {
                lv_canvas_set_px(canvas, xpos, ypos, fillColor, LV_OPA_COVER);
            }
        }

        lastHeight += heightPerEvent;
    }
}

}
