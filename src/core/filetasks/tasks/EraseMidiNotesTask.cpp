// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/filetasks/tasks/EraseMidiNotesTask.h"
#include "common/defines.h"

#include "core/utility/FileWorkerContext.h"
#include "core/primitives/MidiFile.h"
#include "core/primitives/MidiEvent.h"

#include "common/FileIO.h"
#include "common/logger.h"

#include <memory>
#include <algorithm>

namespace slr {

struct File;
struct FileWorkerContext;

namespace Tasks {

void EraseMidiNotes::exec(FileWorkerContext &ctx) {
    if(fileId == 0) {
        LOG_FAIL("FileId can't be zero");
        finished(nullptr, {}, false);
        return;
    }

    File * tmpf = ctx.findFile(fileId);
    MidiFile *file = dynamic_cast<MidiFile*>(tmpf);
    if(!file) {
        LOG_FAIL("Failed to find or convert to Midi File %u", fileId);
        finished(nullptr, {}, false);
        return;
    }

    if(midiTrackId > file->trackCount()) {
        LOG_FAIL("Midi Track Id %u can't be bigger than track count %lu", midiTrackId, file->trackCount());
        finished(nullptr, {}, false);
        return;
    }

    std::unique_ptr<MidiTrack> newbuf = std::make_unique<MidiTrack>();
    const MidiTrack *trk = file->track(midiTrackId);
    
    newbuf->midiEvents.reserve(trk->midiEvents.size()-(notes.size()*2));
    
    // newbuf->midiEvents = trk->midiEvents;
    newbuf->metaEvents = trk->metaEvents;
    newbuf->sysexEvents = trk->sysexEvents;

    std::vector<std::size_t> indexesToSkip;

    auto isNoteEvent = [](const MidiEvent &ev, 
                            const Note &n, 
                            MidiEventType type) -> bool 
    {
        bool t1 = ev.note == n.note;
        bool t2 = ev.velocity == (type == MidiEventType::NoteOn ? n.velocity : 0);
        bool t3 = ev.channel == n.channel;
        bool t4 = ev.offset == (type == MidiEventType::NoteOn ? n.ppqnStart : n.ppqnEnd);
        bool t5 = ev.type == type;
        return t1 && t2 && t3 && t4 && t5;
        // return (
        //     ev.note == n.note &&
        //     ev.velocity == n.velocity &&
        //     ev.channel == n.channel &&
        //     ev.offset == (type == MidiEventType::NoteOn ? n.ppqnStart : n.ppqnEnd) &&
        //     ev.type == type
        // );
    };

    for(const auto &n : notes) {
        //for each note find noteOn and noteOff event
        for(std::size_t i=0; i<trk->midiEvents.size(); ++i) {
            const MidiEvent &ev = trk->midiEvents[i];

            if(isNoteEvent(ev, n, MidiEventType::NoteOn) ||
                isNoteEvent(ev, n, MidiEventType::NoteOff))
            {
                indexesToSkip.push_back(i);
            }
        }
        //add events to indexesToSkip
    }

    if(indexesToSkip.size() % 2 != 0) {
        LOG_FAIL("Count must be devidable by two");
        finished(nullptr, {}, false);
        return;
    }
    
    if(indexesToSkip.size() == 0) {
        LOG_FAIL("Nothing to delete");
        finished(nullptr, {}, false);
        return;
    }

    auto isNotIn = [&list = indexesToSkip](std::size_t &idx) -> bool {
        for(auto &i : list) { if(i == idx) return false; }
        return true;
    };

    //copy data from orig to new, but without indexesToSkip
    for(std::size_t idx=0; idx<trk->midiEvents.size(); ++idx) {
        if(isNotIn(idx)) newbuf->midiEvents.push_back(trk->midiEvents[idx]);
    }

    std::sort(
        newbuf->midiEvents.begin(), 
        newbuf->midiEvents.end(), 
        [](MidiEvent a, MidiEvent b) {
            return a.offset < b.offset;
        });

    finished(file, std::move(newbuf), true);
}

} //namespace Tasks

} //namespace slr
    