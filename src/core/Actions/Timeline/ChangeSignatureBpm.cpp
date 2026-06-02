// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Actions/Timeline/ChangeSignatureBpm.h"

#include "core/primitives/ActionBase.h"
#include "core/primitives/AudioUnit.h"
#include "core/primitives/ControlContext.h"
#include "core/Project.h"
#include "core/Timeline.h"

#include "snapshots/ProjectView.h"
#include "snapshots/TimelineView.h"

#include "ui/uiControls.h"

#include "logger.h"

#include <cassert>
#include <memory>
  
namespace slr {
	
ChangeSignatureBpmAction::ChangeSignatureBpmAction(const ActionBase *base) :
	_action( *(static_cast<const Actions::ChangeSignatureBpm*>(base)) ) {
}
ChangeSignatureBpmAction::~ChangeSignatureBpmAction() {}

void ChangeSignatureBpmAction::exec(ControlContext &ctx) {
	assert(getState() == ActionState::Executing);
	
	switch(_step) {
		case(1): {
            _oldValues.bpm = ctx.project->timeline().bpm();
            _oldValues.sig = ctx.project->timeline().getBarSize();

            _flat.tl = &ctx.project->timeline();
            _flat.bpm = _action.bpm;
            _flat.sig = _action.sig;
            _flat.completed.store(false);

            _task = makeRtTask(&_flat);
            setState(ActionState::Waiting);
            ctx.EmitRtTask(&_task);
        } break;
        case(2): {
            ctx.projectView->timeline().setBpm(_action.bpm);
            ctx.projectView->timeline().setBarSize(_action.sig);
            UIControls::updateTimeline(true);

        	markDelete();
        	setState(ActionState::Finished);
		} break;
        default: assert(false && "Unreachable"); break;
	}
}
void ChangeSignatureBpmAction::checkWaitingCondition(ControlContext &ctx) {
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

void ChangeSignatureBpmAction::ChangeSigBpm::execRT() {
    tl->setBpm(bpm);
    tl->setBarSize(sig);
    completed.store(true, std::memory_order_release);
}

void ChangeSignatureBpmAction::undo(ControlContext &ctx) {
    // Actions::ChangeSignatureBpm tmp = _action;
    // _action = _oldValues;
    // _oldValues = tmp;
    // _step = 1;
    // setState(ActionState::Executing);
    auto act = std::make_unique<Actions::ChangeSignatureBpm>();
    *act = _oldValues;
    EmitAction(std::move(act));
}

void ChangeSignatureBpmAction::redo(ControlContext &ctx) {
    // Actions::ChangeSignatureBpm tmp = _action;
    // _action = _oldValues;
    // _oldValues = tmp;
    // _step = 1;
    // setState(ActionState::Executing);
    auto act = std::make_unique<Actions::ChangeSignatureBpm>();
    *act = _action;
    EmitAction(std::move(act));
}

std::unique_ptr<ActionExecutable> createChangeSignatureBpmAction(const ActionBase*base) {
    return std::make_unique<ChangeSignatureBpmAction>(base);
}


}