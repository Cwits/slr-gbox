// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Actions/Project/CreateNewUnit.h"
#include "core/primitives/ActionBase.h"
#include "core/primitives/AudioUnit.h"
#include "core/primitives/ControlContext.h"

#include "core/Project.h"
#include "core/ModuleManager.h"

#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"

#include "ui/uiControls.h"

#include "logger.h"

#include <cassert>

namespace slr {

CreateNewUnitAction::CreateNewUnitAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::CreateNewUnit*>(base)) )
{
}

CreateNewUnitAction::~CreateNewUnitAction() {

}

void CreateNewUnitAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    const Module *mod = ModuleManagerFactory::findModule(_action.name);
    if(!mod) {
        LOG_ERROR("Failed to find module %s", _action.name);
        abortAction();
        return;
    }

    AudioUnit * au = ctx.project->createUnit(ctx, mod);
    if(!au) {
        LOG_ERROR("Failed to create RT Unit %s", _action.name);
        abortAction();
        return;
    }

    std::shared_ptr<AudioUnitView> view = ctx.projectView->createUnitView(ctx, mod, au);
    if(!view) {
        LOG_ERROR("Failed to create unit view %s", _action.name);
        abortAction();
        return;
    }

    UIControls::addModuleUI(mod, view);

    markDelete();
    setState(ActionState::Finished);
}

void CreateNewUnitAction::checkWaitingCondition() {

}

std::unique_ptr<ActionExecutable> createCreateNewUnitAction(const ActionBase *base) {
    return std::make_unique<CreateNewUnitAction>(base);
}

}
