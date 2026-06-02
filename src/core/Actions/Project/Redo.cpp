// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Actions/Project/Redo.h"

#include "core/primitives/ActionBase.h"
#include "core/primitives/ControlContext.h"

#include "core/Project.h"

#include "snapshots/ProjectView.h"

#include "ui/uiControls.h"

#include "logger.h"

#include <cassert>

namespace slr {

RedoAction::RedoAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::Redo*>(base)) )
{

}

RedoAction::~RedoAction() {

}

void RedoAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    if(ctx._redo->size() > 0) {
        std::unique_ptr<Undoable> a = std::move(ctx._redo->back());
        ctx._redo->pop_back();

        a->redo(ctx);

        // if(ctx._undo->size() >= 64) {
        //     ctx._undo->pop_front();
        // }
        // ctx._undo->push_back(std::move(a));
    }
    markDelete();
    setState(ActionState::Finished);
}

void RedoAction::checkWaitingCondition(ControlContext &ctx) {
    assert(getState() == ActionState::Waiting);
    assert(false && "Unreachable");
}

std::unique_ptr<ActionExecutable> createRedoAction(const ActionBase *base) {
    return std::make_unique<RedoAction>(base);
}

}