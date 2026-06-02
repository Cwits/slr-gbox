// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "units/Track/Actions/DumpRecordedAudio.h"

#include "core/primitives/ActionBase.h"
#include "core/primitives/ControlContext.h"
#include "core/ActionsMap.h"

#include "core/Project.h"
#include "core/FileWorker.h"
#include "core/FileTasks.h"
#include "core/primitives/File.h"
#include "core/primitives/AudioFile.h"
#include "core/utility/basicAudioManipulation.h"
#include "core/BufferManager.h"

#include "core/Actions.h"

#include "units/Track/Track.h"
#include "units/Track/TrackActions.h"

#include "logger.h"

#include <cassert>

namespace slr {

// _actionMap[typeid(Actions::DumpRecordedAudio)] = &createDumpRecAudioAction;

DumpRecAudioAction::DumpRecAudioAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::DumpRecordedAudio*>(base)) )
{

}

DumpRecAudioAction::~DumpRecAudioAction() {

}

void DumpRecAudioAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    LOG_INFO("Dumping audio data to file %s", _action.targetFile->name().c_str());
    auto task = std::make_unique<Tasks::dumpAudio>();
    task->buffer = _action.targetBuffer;
    task->file = _action.targetFile;
    task->size = _action.size;

    task->callback = [ctx, rec = _action.targetBuffer, 
                    target = _action.targetFile, 
                    trackId = _action.trackId,
                    startPos = _action.fileStartPosition]
                    (FileWorker * const fw, bool success) 
    {
        //clean and release buffer no matter what happened
        AudioBuffer * buf = rec;
            
        for(int i=0; i<buf->channels(); ++i) {
            clearAudioBuffer((*buf)[i], buf->bufferSize());
        }
            
        // AudioBufferManager::releaseRecord(buf);
        ctx.bufferManager->releaseAudioRecord(buf);

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

            auto act = std::make_unique<slr::Actions::LoadAsClip>();
            act->data = path;
            act->targetId = trackId;
            act->startOffset = startPos;
            act->makeUnique = true;
            slr::EmitAction(std::move(act));
        }
    };

    ctx.fileWorker->addTask(std::move(task));

    setState(ActionState::Finished);
}

void DumpRecAudioAction::checkWaitingCondition(ControlContext &ctx) {
    assert(getState() == ActionState::Waiting);

}


std::unique_ptr<ActionExecutable> createDumpRecAudioAction(const ActionBase *base) {
    return std::make_unique<DumpRecAudioAction>(base);
}

}
