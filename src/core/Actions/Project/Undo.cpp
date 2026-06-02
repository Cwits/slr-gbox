// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Actions/Project/Undo.h"

#include "core/primitives/ActionBase.h"
#include "core/primitives/ControlContext.h"

#include "core/Project.h"

#include "snapshots/ProjectView.h"

#include "ui/uiControls.h"

#include "logger.h"

#include <cassert>

namespace slr {

UndoAction::UndoAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::Undo*>(base)) )
{

}

UndoAction::~UndoAction() {

}

void UndoAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    if(ctx._undo->size() > 0) {
        std::unique_ptr<Undoable> a = std::move(ctx._undo->back());
        ctx._undo->pop_back();

        a->undo(ctx);

        {
            //block mutex, add event to queue again
        }

        if(ctx._redo->size() >= 64) {
            ctx._redo->pop_front();
        }
        ctx._redo->push_back(std::move(a));
    }
    markDelete();
    setState(ActionState::Finished);
}

void UndoAction::checkWaitingCondition(ControlContext &ctx) {
    assert(getState() == ActionState::Waiting);
    assert(false && "Unreachable");
}

std::unique_ptr<ActionExecutable> createUndoAction(const ActionBase *base) {
    return std::make_unique<UndoAction>(base);
}

}