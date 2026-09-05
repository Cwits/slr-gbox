// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/actions/Project/SaveProject.h"
#include "core/actions/ActionBase.h"

#include "core/Serializer.h"

#include "common/logger.h"

#include <cassert>

namespace slr {

SaveProjectAction::SaveProjectAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::SaveProject*>(base)) )
{
}

SaveProjectAction::~SaveProjectAction() {

}

void SaveProjectAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    if(!Serializer::serialize(ctx)) {
        LOG_ERROR("Failed to save project");
    }

     
    setState(ActionState::Finished);
}

void SaveProjectAction::checkWaitingCondition(ControlContext &ctx) {
    assert(false);
}

std::unique_ptr<ActionExecutable> createSaveProjectAction(const ActionBase *base) {
    return std::make_unique<SaveProjectAction>(base);
}

}
