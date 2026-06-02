// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Actions/AudioUnit/SetColor.h"

#include "core/primitives/ActionBase.h"
#include "core/primitives/ControlContext.h"

#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"

#include "ui/uiControls.h"

#include <cassert>
#include "logger.h"

namespace slr {

SetColorAction::SetColorAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::SetColor*>(base)) )
{

}

SetColorAction::~SetColorAction() {

}

void SetColorAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    
    AudioUnitView *uView = ctx.projectView->getUnitById(_action.targetId);
    if(!uView) {
        LOG_ERROR("Failed to find view for ID %u", _action.targetId);
        abortAction();
        return;
    }

    uView->color(_action.color);

    setState(ActionState::Finished);
}

void SetColorAction::checkWaitingCondition(ControlContext &ctx) {
    assert(getState() == ActionState::Waiting);
    assert(false && "Unreachable");
}

std::unique_ptr<ActionExecutable> createSetColorAction(const ActionBase *base) {
    return std::make_unique<SetColorAction>(base);
}

}