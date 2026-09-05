// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/actions/AudioUnit/SetName.h"
#include "core/actions/ActionBase.h"
#include "core/utility/ControlContext.h"

#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"

#include "common/logger.h"

#include <cassert>

namespace slr {


SetNameAction::SetNameAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::SetName*>(base)))
{
}

SetNameAction::~SetNameAction() {

}

void SetNameAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    AudioUnitView *view = ctx.projectView->getUnitById(_action.targetId);
    if(!view) {
        LOG_ERROR("Failed to find unit");
        abortAction();
        return;
    }

    std::string nameToSet;

    if(_direction == ActionDirection::Forward) {
        nameToSet = _action.newName;
        _oldName = view->name();
    } else {
        nameToSet = _oldName;
    }

    view->setName(_action.newName);

    setState(ActionState::Finished);
}

void SetNameAction::checkWaitingCondition(ControlContext &ctx) {
    assert(getState() == ActionState::Waiting);
    assert(false);
}

std::unique_ptr<ActionExecutable> createSetNameAction(const ActionBase*base) {
    return std::make_unique<SetNameAction>(base);
}

}
