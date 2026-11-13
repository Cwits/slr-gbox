// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/filetasks/tasks/UpdateAudioPeaks.h"
#include "common/defines.h"

#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioPeakFile.h"
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

void UpdateAudioPeaks::exec(FileWorkerContext &ctx) {
    if(audioFileId == 0) {
        LOG_FAIL("File ID can't be zero");
        finished(false);
        return;
    }

    File * tmpa = ctx.findFile(audioFileId);
    AudioFile * afile = dynamic_cast<AudioFile*>(tmpa);
    if(!afile) {
        LOG_FAIL("File not an audio file");
        finished(false);
        return;
    }

    File * tmpapk = ctx.findFile(afile->peakFileId());
    AudioPeakFile *apk = dynamic_cast<AudioPeakFile*>(tmpapk);
    if(!apk) {
        LOG_FAIL("Failed to convert to peak file");
        finished(false);
        return;
    }

    apk->peaks()->build(afile);
    finished(true);
}

} //namespace Tasks

} //namespace slr
    