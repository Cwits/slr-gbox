// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/filetasks/tasks/AddNewMidiNoteTask.h"
#include "common/defines.h"

#include "core/utility/FileWorkerContext.h"
#include "core/primitives/MidiFile.h"

#include "common/FileIO.h"
#include "common/logger.h"

#include <memory>
#include <algorithm>

namespace slr {

struct File;
struct FileWorkerContext;

namespace Tasks {

void AddNewMidiNote::exec(FileWorkerContext &ctx) {
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

    newbuf->midiEvents.reserve(trk->midiEvents.size()+2);
    
    newbuf->midiEvents = trk->midiEvents;
    newbuf->metaEvents = trk->metaEvents;
    newbuf->sysexEvents = trk->sysexEvents;

    MidiEvent newevent;
    newevent.velocity = note.velocity;
    newevent.channel = note.channel;
    newevent.type = MidiEventType::NoteOn;
    newevent.note = note.note;
    newevent.offset = note.ppqnStart;

    newbuf->midiEvents.push_back(newevent);

    newevent.offset = note.ppqnEnd;
    newevent.type = MidiEventType::NoteOff;

    newbuf->midiEvents.push_back(newevent);

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
    