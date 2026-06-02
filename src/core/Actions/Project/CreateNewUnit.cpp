// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Actions/Project/CreateNewUnit.h"
#include "core/primitives/ActionBase.h"
#include "core/primitives/AudioUnit.h"
#include "core/primitives/ControlContext.h"

#include "core/Project.h"
#include "core/UnitManager.h"
#include "core/Actions.h"

#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"

#include "ui/uiControls.h"

#include "logger.h"

#include <cassert>

namespace slr {

CreateNewUnitAction::CreateNewUnitAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::CreateNewUnit*>(base)) )
{
    _createdUnitId = 0;
    _view = nullptr;
}

CreateNewUnitAction::~CreateNewUnitAction() {

}

void CreateNewUnitAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);
    
    const UnitDescriptor *desc = UnitManagerFactory::findUnit(_action.name);
    if(!desc) {
        LOG_ERROR("Failed to find module %s", _action.name.c_str());
        abortAction();
        return;
    }

    ID nextId = 0;
    if(!_action.forcedId) {
        nextId = ctx.project->getNextUnitId();
    } else {
        nextId = _action.forcedId.value();
    }

    AudioUnit * au = ctx.project->createUnit(ctx.bufferManager, desc, nextId);
    if(!au) {
        LOG_ERROR("Failed to create RT Unit %s", _action.name.c_str());
        abortAction();
        return;
    }

    std::shared_ptr<AudioUnitView> view = ctx.projectView->createUnitView(ctx, desc, au);
    if(!view) {
        LOG_ERROR("Failed to create unit view %s", _action.name.c_str());
        abortAction();
        return;
    }

    _view = view.get();
    UIControls::addUnitUI(desc, view);

    _createdUnitId = au->id();

    markDelete();
    setState(ActionState::Finished);
}

void CreateNewUnitAction::checkWaitingCondition(ControlContext &ctx) {
    assert(getState() == ActionState::Waiting);
}

void CreateNewUnitAction::undo(ControlContext &ctx) {
    auto act = std::make_unique<Actions::DeleteUnit>();
    act->targetId = _createdUnitId;
    EmitAction(std::move(act));
}

void CreateNewUnitAction::redo(ControlContext &ctx) {
    auto act = std::make_unique<Actions::CreateNewUnit>();
    *act = _action;
    EmitAction(std::move(act));
    // setState(ActionState::Executing);
}

std::unique_ptr<ActionExecutable> createCreateNewUnitAction(const ActionBase *base) {
    return std::make_unique<CreateNewUnitAction>(base);
}

}
