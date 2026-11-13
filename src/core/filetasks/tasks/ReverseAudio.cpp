// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/filetasks/tasks/ReverseAudio.h"
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

void ReverseAudio::exec(FileWorkerContext &ctx) {
    if(start > end) {
        LOG_FAIL("Start can't be bigger than end");
        finished(nullptr, std::unique_ptr<AudioBuffer>(), false);
        return;
    }

    File * tmpa = ctx.findFile(fileId);
    AudioFile * afile = dynamic_cast<AudioFile*>(tmpa);
    if(!afile) {
        LOG_FAIL("File not an audio file");
        finished(nullptr, std::unique_ptr<AudioBuffer>(), false);
        return;
    }

    //need to check that end < file length?

    int channels = afile->channels();
    
    frame_t total = afile->frames();
    std::unique_ptr<AudioBuffer> newbuf = std::make_unique<AudioBuffer>(total, channels);
    for(int ch=0; ch<channels; ++ch) {
        std::memcpy((*newbuf.get())[ch], (*afile->data())[ch], total*sizeof(sample_t));
    }

    frame_t diff = end - start;
    for(int ch=0; ch<channels; ++ch) {
        const sample_t *smpRead = &(*afile->data())[ch][start];
        sample_t *smpWrite = &(*newbuf)[ch][start];

        for(frame_t f=0; f<diff; ++f) {
            smpWrite[f] = smpRead[end-f];
        }
    }

    finished(afile, std::move(newbuf), true);
}

} //namespace Tasks

} //namespace slr
    