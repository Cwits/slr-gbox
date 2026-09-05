// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/actions/AudioUnit/SetColor.h"

#include "core/actions/ActionBase.h"
#include "core/utility/ControlContext.h"

#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"

#include "common/uiControls.h"

#include <cassert>
#include "common/logger.h"

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
    
    Color clrToSet;

    if(_direction == ActionDirection::Forward) { 
        _oldColor = uView->color();
        clrToSet = _action.color;
    } else {
        clrToSet = _oldColor;
    }
    
    uView->color(clrToSet);

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