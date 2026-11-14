// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/actions/FileTaskProxy/AddNewMidiNote.h"

#include "core/actions/ActionBase.h"
#include "core/utility/ControlContext.h"

#include "core/primitives/MidiFile.h"
#include "core/filetasks/FileTasks.h"
#include "core/FileWorker.h"
#include "core/Project.h"

#include "snapshots/ProjectView.h"

#include "common/uiControls.h"

#include "common/logger.h"

#include <cassert>

namespace slr {

AddNewMidiNoteAction::AddNewMidiNoteAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::AddNewMidiNote*>(base)) )
{

}

AddNewMidiNoteAction::~AddNewMidiNoteAction() {

}

void AddNewMidiNoteAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    switch(_step) {
    	case(1): {
            auto task = std::make_unique<Tasks::AddNewMidiNote>();
            task->note = _action.note;
            task->fileId = _action.fileId;
            task->midiTrackId = _action.midiTrackId;
            task->finished = [&fin = _finished, &edit = _editable, &swap = _swapable](MidiFile *f, std::unique_ptr<MidiTrack> buf, bool success) {
                if(success) {
                    edit = f;
                    swap = std::move(buf);
                }
                fin.store(true, std::memory_order_release);
            };
            _finished.store(false, std::memory_order_release);
            
            ctx.project->fileWorker()->addTask(std::move(task));
            setState(ActionState::Waiting);
    	} break;
    	case(2): {
            if(!_swapable || !_editable) {
                LOG_FAIL("Failed to prepare buffer to swap");
                abortAction();
                return;
            }

            _flat.file = _editable;
            _flat.swappable = std::move(_swapable);
            _flat.trackId = _action.midiTrackId;
            _flat.completed.store(false);
            _task = makeRtTask(&_flat);
            
            setState(ActionState::Waiting);
            ctx.EmitRtTask(&_task);

            setState(ActionState::Waiting);
    	} break;
        case(3): {
            //update peaks
            auto task = std::make_unique<Tasks::UpdateMidiPeaks>();
            task->midiFileId = _action.fileId;
            task->midiTrackId = _action.midiTrackId;
            task->finished = [&fin = _finished](bool success) {
                fin.store(true, std::memory_order_release);
            };
            _finished.store(false, std::memory_order_release);
            ctx.project->fileWorker()->addTask(std::move(task));
            setState(ActionState::Waiting);

        } break;
        case(4): {
            if(_action.finished) _action.finished();
            setState(ActionState::Finished);
        } break;
        default: assert(false && "Unreachable"); break;
    }
}

void AddNewMidiNoteAction::checkWaitingCondition(ControlContext &ctx) {
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

void AddNewMidiNoteAction::SwapMidiData::execRT() {
    file->swapTrack(trackId, swappable);
    completed.store(true, std::memory_order_release);
}

std::unique_ptr<ActionExecutable> createAddNewMidiNoteAction(const ActionBase *base) {
    return std::make_unique<AddNewMidiNoteAction>(base);
}

}
