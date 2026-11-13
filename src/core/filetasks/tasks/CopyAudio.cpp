// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/filetasks/tasks/CopyAudio.h"
#include "common/defines.h"

#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioPeakFile.h"
#include "core/primitives/AudioBuffer.h"
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

void CopyAudio::exec(FileWorkerContext &ctx) {
    if(start > end) {
        LOG_FAIL("Start can't be bigger than end");
        if(finished) finished(nullptr, false);
        return;
    }

    if(fileId == 0) {
        LOG_FAIL("Wrong file id");
        if(finished) finished(nullptr, false);
        return;
    }

    const File *tmp = ctx.worker->findFileById(fileId);
    if(!tmp) {
        LOG_FAIL("Unable to find file with id %u", fileId);
        if(finished) finished(nullptr, false);
        return;
    }

    const AudioFile * file = dynamic_cast<const AudioFile*>(tmp);
    if(!file) {
        LOG_FAIL("Failed to convert to audio file %s", tmp->name().c_str());
        if(finished) finished(nullptr, false);
        return;
    }

    // int ch = channel ? channel.value() : raw->channels();
    if(channel) {
        //pff... need to copy specific channel of audio
        LOG_WARN("Specific channel copying not working yet");
    } else {

    }

    
    const AudioBuffer * raw = file->data();
    AudioCopyContext *actx = ctx._aCopyCtx;
    if(actx->_data) actx->_data.reset();
    actx->originFileId = fileId;
    actx->originPath = file->path();

    slr::frame_t length = end - start;
    // length % file->channels() != 0 ? length -= 1 : 0;
    actx->_data = std::make_unique<AudioBuffer>(length, file->channels());


    //not like this, data in raw is uninterleaved, so it is LLLLLLLL...RRRRRRRR
    // std::memcpy(actx->_data.get(), raw, sizeof(sample_t)*length);
    // std::memcpy(actx->_data)
    for(int ch=0; ch<file->channels(); ++ch) {
        AudioBuffer *dst = actx->_data.get();
        std::memcpy((*dst)[ch], &(*raw)[ch][start], length*sizeof(sample_t));
    }
    LOG_SUCCESS("Successfully copied audio from %s to buffer", file->name().c_str());
    if(finished) finished(file, true);
}

} //namespace Tasks

} //namespace slr
    