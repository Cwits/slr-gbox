// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/filetasks/tasks/SilenceAudio.h"
#include "common/defines.h"

#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioPeakFile.h"
#include "core/utility/FileWorkerContext.h"
#include "core/FileWorker.h"
#include "core/Project.h"

#include "common/FileIO.h"
#include "common/logger.h"

#include <memory>
#include <algorithm>

namespace slr {

struct File;
struct FileWorkerContext;

namespace Tasks {

void SilenceAudio::exec(FileWorkerContext &ctx) {
    if(start > end) {
        LOG_FAIL("Start can't be bigger than end");
        finished(nullptr, std::unique_ptr<AudioBuffer>());
        return;
    }

    if(!ctx._aCopyCtx->_data) {
        LOG_FAIL("Copy buffer is empty");
        finished(nullptr, std::unique_ptr<AudioBuffer>());
        return;
    }

    File * tmpa = ctx.findFile(fileId);
    AudioFile * afile = dynamic_cast<AudioFile*>(tmpa);
    if(!afile) {
        LOG_FAIL("File not an audio file");
        finished(nullptr, std::unique_ptr<AudioBuffer>());
        return;
    }
    
    int channels = afile->channels();
    frame_t total = end - start;

    std::unique_ptr<AudioBuffer> newbuf = std::make_unique<AudioBuffer>(afile->frames(), channels);
    for(int ch=0; ch<channels; ++ch) {
        std::memset(&(*newbuf.get())[ch][start], 0, total*sizeof(sample_t));
    }

    finished(afile, std::move(newbuf));
}

} //namespace Tasks

} //namespace slr
    