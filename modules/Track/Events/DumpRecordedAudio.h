// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK DumpRecordedAudio

FLAT_RESP
INCLUDE "core/primitives/AudioBuffer.h"
INCLUDE "defines.h"
struct DumpRecordedAudio {
    AudioBuffer * targetBuffer;
    AudioFile * targetFile; //-> class AudioFile;
    frame_t size;
    frame_t fileStartPosition;
    ID trackId;
};

RESP_HANDLE
INCLUDE "modules/Track/Track.h"
INCLUDE "core/FileWorker.h"
INCLUDE "core/FileTasks.h"
INCLUDE "core/primitives/AudioBuffer.h"
INCLUDE "core/utility/basicAudioManipulation.h"
INCLUDE "core/primitives/File.h"
INCLUDE "core/primitives/AudioFile.h"
INCLUDE "logger.h"
INCLUDE <string>
void handleDumpRecordedAudio(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
    LOG_INFO("Dumping audio data to file %s", resp.dumpRecordedAudio.targetFile->name().c_str());
    auto task = std::make_unique<Tasks::dumpAudio>();
    task->buffer = resp.dumpRecordedAudio.targetBuffer;
    task->file = resp.dumpRecordedAudio.targetFile;
    task->size = resp.dumpRecordedAudio.size;

    task->callback = [rec = resp.dumpRecordedAudio.targetBuffer, 
                    target = resp.dumpRecordedAudio.targetFile, 
                    trackId = resp.dumpRecordedAudio.trackId,
                    startPos = resp.dumpRecordedAudio.fileStartPosition]
                    (FileWorker * fw, bool success) 
    {
        //clean and release buffer no matter what happened
        AudioBuffer * buf = rec;
            
        for(int i=0; i<buf->channels(); ++i) {
            clearAudioBuffer((*buf)[i], buf->bufferSize());
        }
            
        AudioBufferManager::releaseRecord(buf);

        if(!success) {
            LOG_ERROR("Failed to dump audio data to file");
            return;
        }
        
        LOG_INFO("audio data dumped successfully to file %s", target->name().c_str());
            
        if(target->finalize()) {
            LOG_INFO("Finalizing audio file %s path %s", 
                target->name().c_str(), 
                target->path().c_str());
            // LOG_WARN("Finalizing audio file not implemented");
            //finalize file and return it to track
            std::string path = target->path();

            fw->closeTmpAudioFile(target);

            slr::Events::OpenFile e = {
                .unitId = trackId,
                .path = path,
                .fileStartPosition = startPos
            };
            slr::EmitEvent(e);
        }
    };

    ctx.fileWorker->addTask(std::move(task));
}
END_HANDLE

END_BLOCK