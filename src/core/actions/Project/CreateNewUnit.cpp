// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/actions/Project/CreateNewUnit.h"
#include "core/actions/ActionBase.h"
#include "core/primitives/AudioUnit.h"
#include "core/utility/ControlContext.h"

#include "core/Project.h"
#include "core/UnitManager.h"
#include  "core/actions/Actions.h"

#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"

#include "common/uiControls.h"

#include "common/logger.h"

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
    
    if(_direction == ActionDirection::Forward) {
        const UnitDescriptor *desc = UnitManagerFactory::findUnit(_action.name);
        if(!desc) {
            LOG_ERROR("Failed to find module %s", _action.name.c_str());
            abortAction();
            return;
        }

        ID nextId = 0;
        if(!_action.forcedId) {
            if(_createdUnitId == 0)
                nextId = ctx.project->getNextUnitId();
            else 
                nextId = _createdUnitId;    
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

        setState(ActionState::Finished);
    } else {
        UIControls::removeUI(_createdUnitId);

        std::unique_ptr<AudioUnit> unit = ctx.project->removeUnit(_createdUnitId);
        unit->destroy(ctx.bufferManager);

        std::shared_ptr<AudioUnitView> uview = ctx.projectView->removeUnitView(_createdUnitId);

        UIControls::deleteUI(_createdUnitId); //possible dangling unit ui??


        setState(ActionState::Finished);
    }
}

void CreateNewUnitAction::checkWaitingCondition(ControlContext &ctx) {
    assert(getState() == ActionState::Waiting);
}

std::unique_ptr<ActionExecutable> createCreateNewUnitAction(const ActionBase *base) {
    return std::make_unique<CreateNewUnitAction>(base);
}

}
