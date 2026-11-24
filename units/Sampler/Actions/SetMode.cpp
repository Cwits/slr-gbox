// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "units/Sampler/Actions/SetMode.h"

#include "units/Sampler/Sampler.h"
#include "units/Sampler/SamplerView.h"

#include "core/actions/ActionBase.h"
#include "core/primitives/AudioFile.h"
#include "core/utility/ControlContext.h"
#include "core/FileWorker.h"
#include "core/filetasks/FileTasks.h"
#include "common/FileIO.h"

#include "core/Project.h"
#include "snapshots/ProjectView.h"
// #include "snapshots/AudioUnitView.h"

#include "common/logger.h"

#include <cassert>

namespace slr {

SetModeAction::SetModeAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::SetMode*>(base)) )
{

}

SetModeAction::~SetModeAction() {

}

void SetModeAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    switch(_step) {
    	case(1): {
            AudioUnit *au = ctx.project->getUnitById(_action.targetId);
            Sampler * smp = dynamic_cast<Sampler*>(au);

            if(!smp) {
                LOG_FAIL("Failed to find sampler object from id %u", _action.targetId);
                abortAction();
                return;
            }

            _sampler = smp;

            if(_action.newMode == SamplerMode::Slicer) {
                //need to prepare slices
            }
            
            _step = 2;
        } break;
        case(2): {
            

            _flat.sampler = _sampler;
            _flat.mode = _action.newMode;
            _flat.completed.store(false);
            _task = makeRtTask(&_flat);
            
            setState(ActionState::Waiting);
            ctx.EmitRtTask(&_task);
    	} break;
    	case(3): {
            AudioUnitView *auv = ctx.projectView->getUnitById(_action.targetId);
            SamplerView *smpv = dynamic_cast<SamplerView*>(auv);

            if(!smpv) {
                LOG_FAIL("Failed to find view for sampler id %u", _action.targetId);
                abortAction();
                return;
            }

            smpv->mode(_action.newMode);

            setState(ActionState::Finished);
    	} break;
        default: assert(false && "Unreachable"); break;
    }
}

void SetModeAction::checkWaitingCondition(ControlContext &ctx) {
    assert(getState() == ActionState::Waiting);

    switch(_step) {
        case(1): {
        } break;
        case(2): {
            bool res = _flat.completed.load(std::memory_order_acquire);
            if(res) {
                _step = 3;
                setState(ActionState::Executing);
            }
        } break;
        default: assert(false && "Unreachable"); break;
    }
}


void SetModeAction::SetModeFlat::execRT() {
    sampler->clearAllVoices();
    sampler->mode(mode);
    completed.store(true, std::memory_order_release);
}

std::unique_ptr<ActionExecutable> createSetModeAction(const ActionBase *base) {
    return std::make_unique<SetModeAction>(base);
}

}