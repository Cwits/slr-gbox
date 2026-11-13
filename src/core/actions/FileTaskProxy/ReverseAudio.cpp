// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/actions/FileTaskProxy/ReverseAudio.h"

#include "core/actions/ActionBase.h"
#include "core/utility/ControlContext.h"

#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioPeaks.h"
#include "core/Project.h"
#include "core/FileWorker.h"
#include "core/filetasks/FileTasks.h"

#include "snapshots/ProjectView.h"

#include "common/uiControls.h"

#include "common/logger.h"

#include <cassert>

namespace slr {

ReverseAudioAction::ReverseAudioAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::ReverseAudio*>(base)) )
{

}

ReverseAudioAction::~ReverseAudioAction() {

}

void ReverseAudioAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    switch(_step) {
    	case(1): {
            auto fact = std::make_unique<Tasks::ReverseAudio>();
            fact->start = _action.start;
            fact->end = _action.end;
            fact->channel = _action.channel;
            fact->fileId = _action.fileId;
            _finished.store(false, std::memory_order_release);

            fact->finished = [&swap = _swapTarget, &fin = _finished, &tgt = _readyBuffer](AudioFile *target, std::unique_ptr<AudioBuffer> buf, bool success) {
                swap = target;
                tgt = std::move(buf);
                fin.store(true, std::memory_order_release);
            };

            ctx.project->fileWorker()->addTask(std::move(fact));
            setState(ActionState::Waiting);
    	} break;
    	case(2): {
            if(!_readyBuffer || !_swapTarget) {
                LOG_FAIL("Paste action failed");
                abortAction();
                return;
            }
            _flat.file = _swapTarget;
            _flat.buf = std::move(_readyBuffer);
            _flat.completed.store(false);

            _task = makeRtTask(&_flat);
            
            LOG_INFO("Preparing to swap data buffers");

            setState(ActionState::Waiting);
            ctx.EmitRtTask(&_task);
    	} break;
        case(3): {
            //update peaks
            auto act = std::make_unique<Tasks::UpdateAudioPeaks>();
            act->audioFileId = _action.fileId;
            act->start = _action.start;
            act->end = _action.end;
            _finished.store(false, std::memory_order_release);
            act->finished = [&fin = _finished](bool success) {
                fin.store(true, std::memory_order_release);
            };
            ctx.project->fileWorker()->addTask(std::move(act));
            setState(ActionState::Waiting);
        } break;
        case(4): {
            //update version counters?
            LOG_SUCCESS("Finished audio reverse task");
            if(_action.finished) _action.finished();
            setState(ActionState::Finished);
        } break;
        default: assert(false && "Unreachable"); break;
    }
}

void ReverseAudioAction::checkWaitingCondition(ControlContext &ctx) {
    assert(getState() == ActionState::Waiting);

    switch(_step) {
        case(1): {
            bool res = _finished.load(std::memory_order_acquire);
            if(res) {
                _step = 2;
                setState(ActionState::Executing);
            }
        } break;
        case(2): {
            bool res = _flat.completed.load(std::memory_order_acquire);
            if(res) {
                _step = 3;
                setState(ActionState::Executing);
            }
        } break;
        case(3): {
            bool res = _finished.load(std::memory_order_acquire);
            if(res) {
                _step = 4;
                setState(ActionState::Executing);
            }
        } break;
        default: assert(false && "Unreachable"); break;
    }
}

void ReverseAudioAction::SwapAudioFileData::execRT() {
    file->swapData(buf);
    completed.store(true, std::memory_order_release);
}

std::unique_ptr<ActionExecutable> createReverseAudioAction(const ActionBase *base) {
    return std::make_unique<ReverseAudioAction>(base);
}

}
