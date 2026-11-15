// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/actions/FileTaskProxy/CutAudio.h"

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

CutAudioAction::CutAudioAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::CutAudio*>(base)) )
{

}

CutAudioAction::~CutAudioAction() {

}

void CutAudioAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    switch(_step) {
        case(1): {
            auto act = std::make_unique<Tasks::CopyAudio>();
            act->start = _action.start;
            act->end = _action.end;
            act->channel = _action.channel;
            act->fileId = _action.fileId;
            act->finished = [&cmp = _completed](const File * file, bool success) {
                cmp.store(true, std::memory_order_release);
            };
            _completed.store(false, std::memory_order_release);

            ctx.project->fileWorker()->addTask(std::move(act));
            setState(ActionState::Waiting);
        } break;
    	case(2): {
            auto act = std::make_unique<Tasks::DeleteAudio>();
            act->start = _action.start;
            act->end = _action.end;
            act->channel = _action.channel;
            act->fileId = _action.fileId;
            act->finished = [&swap = _swapTarget, &newbuf = _newBuffer, &cmp = _completed](AudioFile *target, std::unique_ptr<AudioBuffer> buf, bool success){
                swap = target;
                newbuf = std::move(buf);
                cmp.store(true, std::memory_order_release);
            };
            _completed.store(false, std::memory_order_release);

            ctx.project->fileWorker()->addTask(std::move(act));
            setState(ActionState::Waiting);
    	} break;
    	case(3): {
            if(!_swapTarget || ! _newBuffer) {
                LOG_FAIL("Failed to prepare swapable buffer for %u", _action.fileId);
                abortAction();
                return;
            }
            
            _flat.file = _swapTarget;
            _flat.clips = ctx.project->clipStorage().clipsRelatedToAudio(_swapTarget->data());
            _flat.buf = std::move(_newBuffer);
            _flat.completed.store(false);

            _task = makeRtTask(&_flat);
            
            LOG_INFO("Preparing to swap data buffers");

            ctx.EmitRtTask(&_task);
            setState(ActionState::Waiting);
    	} break;
        case(4): {
            //update peaks
            auto act = std::make_unique<Tasks::UpdateAudioPeaks>();
            act->audioFileId = _action.fileId;
            act->start = _action.start;
            act->end = _action.end;
            _completed.store(false, std::memory_order_release);
            act->finished = [&fin = _completed](bool success) {
                fin.store(true, std::memory_order_release);
            };
            ctx.project->fileWorker()->addTask(std::move(act));
            setState(ActionState::Waiting);
        } break;
        case(5): {
            LOG_SUCCESS("Finished audio delete task");
            if(_action.finished) _action.finished();
            setState(ActionState::Finished);
        } break;
        default: assert(false && "Unreachable"); break;
    }
}

void CutAudioAction::checkWaitingCondition(ControlContext &ctx) {
    assert(getState() == ActionState::Waiting);

    switch(_step) {
        case(1): {
            bool res = _completed.load(std::memory_order_acquire);
            if(res) {
                _step = 2;
                setState(ActionState::Executing);
            }
        } break;
        case(2): {
            bool res = _completed.load(std::memory_order_acquire);
            if(res) {
                _step = 3;
                setState(ActionState::Executing);
            }
        } break;
        case(3): {
            bool res = _flat.completed.load(std::memory_order_acquire);
            if(res) {
                _step = 4;
                setState(ActionState::Executing);
            }
        } break;
        case(5): {
            bool res = _completed.load(std::memory_order_acquire);
            if(res) {
                _step = 4;
                setState(ActionState::Executing);
            }
        } break;
        default: assert(false && "Unreachable"); break;
    }
}

std::unique_ptr<ActionExecutable> createCutAudioAction(const ActionBase *base) {
    return std::make_unique<CutAudioAction>(base);
}

}
