// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Actions/Timeline/ChangeTimelineState.h"

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
	
ChangeTimelineStateAction::ChangeTimelineStateAction(const ActionBase *base) :
	_action( *(static_cast<const Actions::ChangeTimelineState*>(base)) ) {
}
ChangeTimelineStateAction::~ChangeTimelineStateAction() {}

void ChangeTimelineStateAction::exec(ControlContext &ctx) {
	assert(getState() == ActionState::Executing);
	
	switch(_step) {
		case(1): {
			_flat.tl = &ctx.project->timeline();
			_flat.state = _action.state;
			_flat.completed.store(false);

			_task = makeRtTask(&_flat);
			setState(ActionState::Waiting);

			ctx.EmitRtTask(&_task);
		} break;
		case(2): {
			ctx.projectView->timeline().update();
			UIControls::updateTimeline(false);

        	markDelete();
        	setState(ActionState::Finished);
		} break;
        default: assert(false && "Unreachable"); break;
	}
}
void ChangeTimelineStateAction::checkWaitingCondition() {
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


void ChangeTimelineStateAction::ChangeTimelineState::execRT() {
	tl->setTimelineState(state);
	completed.store(true, std::memory_order_release);
}

std::unique_ptr<ActionExecutable> createChangeTimelineStateAction(const ActionBase*base) {
    return std::make_unique<ChangeTimelineStateAction>(base);
}


}