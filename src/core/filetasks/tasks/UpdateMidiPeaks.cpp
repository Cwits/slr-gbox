// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/filetasks/tasks/UpdateMidiPeaks.h"
#include "common/defines.h"

#include "core/primitives/MidiFile.h"
#include "core/primitives/MidiPeaks.h"
#include "core/utility/FileWorkerContext.h"
#include "core/FileWorker.h"
#include "core/Project.h"

#include "common/FileIO.h"
#include "common/logger.h"

#include <memory>

namespace slr {

struct File;
struct FileWorkerContext;

namespace Tasks {

void UpdateMidiPeaks::exec(FileWorkerContext &ctx) {
    if(midiFileId == 0) {
        LOG_FAIL("File ID can't be zero");
        finished(false);
        return;
    }

    File * tmpa = ctx.findFile(midiFileId);
    MidiFile * afile = dynamic_cast<MidiFile*>(tmpa);
    if(!afile) {
        LOG_FAIL("File not an midi file");
        finished(false);
        return;
    }

    MidiPeaks *pk = afile->peaks(midiTrackId);
    pk->buildPeaks(afile->track(midiTrackId), midiTrackId);
    finished(true);
}

} //namespace Tasks

} //namespace slr
    