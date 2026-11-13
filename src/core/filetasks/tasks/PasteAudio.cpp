// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/filetasks/tasks/PasteAudio.h"
#include "common/defines.h"

#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioPeakFile.h"
#include "core/utility/FileWorkerContext.h"
#include "core/FileWorker.h"
#include "core/Project.h"

#include "common/FileIO.h"
#include "common/logger.h"

#include <memory>
#include <cstring>

namespace slr {

struct File;
struct FileWorkerContext;

namespace Tasks {

void PasteAudio::exec(FileWorkerContext &ctx) {
    if(start > end) {
        LOG_FAIL("Start can't be bigger than end");
        finished(nullptr, std::unique_ptr<AudioBuffer>(), false);
        return;
    }

    if(!ctx._aCopyCtx->_data) {
        LOG_FAIL("Copy buffer is empty");
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
    // start % channels != 0 ? start -= 1 : 0;
    // end % channels != 0 ? end -= 1 : 0;

    frame_t beforeSize = start;
    // beforeSize % channels != 0 ? beforeSize -= 1 : 0;
    // beforeSize /= channels;
    // frame_t afterSize = (afile->frames()*channels) - end;
    frame_t afterSize = afile->frames() - end;
    // afterSize % channels != 0 ? afterSize -= 1 : 0;
    // afterSize /= channels;

    const AudioBuffer *raw = afile->data();
    std::unique_ptr<AudioBuffer> bbuf = std::make_unique<AudioBuffer>(beforeSize, channels);
    std::unique_ptr<AudioBuffer> abuf = std::make_unique<AudioBuffer>(afterSize, channels);

    for(int ch=0; ch<channels; ++ch) {
        std::memcpy((*bbuf.get())[ch], (*raw)[ch], beforeSize*sizeof(sample_t));
        std::memcpy((*abuf.get())[ch], &(*raw)[ch][end], afterSize*sizeof(sample_t));
    }
    
    frame_t total = beforeSize + afterSize + ctx._aCopyCtx->_data->size();
    std::unique_ptr<AudioBuffer> newbuf = std::make_unique<AudioBuffer>(total, channels);
    for(int ch=0; ch<channels; ++ch) {
        std::memcpy((*newbuf.get())[ch], (*bbuf.get())[ch], beforeSize*sizeof(sample_t));
        std::memcpy(&(*newbuf.get())[ch][beforeSize], (*ctx._aCopyCtx->_data.get())[ch], ctx._aCopyCtx->_data->size()*sizeof(sample_t));
        std::memcpy(&(*newbuf.get())[ch][beforeSize+ctx._aCopyCtx->_data->size()], (*abuf.get())[ch], afterSize*sizeof(sample_t));
    }

    finished(afile, std::move(newbuf), true);
}

} //namespace Tasks

} //namespace slr
    