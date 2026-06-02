// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Actions/Project/ModifyClipItem.h"

#include "core/primitives/ActionBase.h"
#include "core/primitives/ControlContext.h"

#include "core/Project.h"
#include "core/primitives/FileContainer.h"

#include "snapshots/ProjectView.h"
#include "snapshots/FileContainerView.h"

#include "ui/uiControls.h"

#include "logger.h"

#include <cassert>

namespace slr {

ModifyClipItemAction::ModifyClipItemAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::ModifyClipItem*>(base)) )
{
}

ModifyClipItemAction::~ModifyClipItemAction() {

}

void ModifyClipItemAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    switch(_step) {
    	case(1): {
            ClipItem * item = ctx.project->findClipItemById(_action.clipId);
            if(!item) {
                LOG_ERROR("Failed to find clip item %u", _action.clipId);
                abortAction();
                return;
            }

            _flat.item = item;
            _flat.startPosition = _action.startPosition;
            _flat.length = _action.length;
            _flat.fileStartOffset = _action.fileStartOffset; 
            _flat.muted = _action.muted;   
            _flat.completed.store(false);

            _task = makeRtTask(&_flat);

            setState(ActionState::Waiting);
            ctx.EmitRtTask(&_task);
        } break;
        case(2): {
            ClipItemView *item = ctx.projectView->findClipViewById(_action.clipId);
            if(!item) {
                LOG_ERROR("Failed to find ClipItemView with id %lu", _action.clipId);
                abortAction();
                return;
            }

            //TODO: update with values from action or flat task...
            item->update(_action.startPosition, _action.length, _action.fileStartOffset, _action.muted);

             
            setState(ActionState::Finished);
        } break;
        default: assert(false && "Unreachable"); break;
    }   
}

void ModifyClipItemAction::checkWaitingCondition(ControlContext &ctx) {
    assert(getState() == ActionState::Waiting);

    switch(_step) {
        case(1): {
            bool res = _flat.completed.load(std::memory_order_acquire);
            if(res) {
                _step = 2;
                setState(ActionState::Executing);
            }
        } break;
        default: assert(false && "Unreachable"); break;
    }
}


void ModifyClipItemAction::ModifyClip::execRT() {
    item->update(startPosition, length, fileStartOffset, muted);
    completed.store(true, std::memory_order_release);
}

std::unique_ptr<ActionExecutable> createModifyClipItemAction(const ActionBase *base) {
    return std::make_unique<ModifyClipItemAction>(base);
}

}
