// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "units/Track/Actions/RecordArm.h"
#include "units/Track/Track.h"
#include "units/Track/TrackView.h"

#include "core/primitives/ActionBase.h"
#include "core/primitives/ControlContext.h"

#include "core/drivers/AudioDriver.h"
#include "core/SettingsManager.h"
#include "core/RtEngine.h"
#include "core/Project.h"
#include "core/utility/helper.h"

#include "snapshots/ProjectView.h"

#include "logger.h"

#include <cassert>
#include <cmath>

namespace slr {

RecordArmAction::RecordArmAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::RecordArm*>(base)) )
{

}

RecordArmAction::~RecordArmAction() {

}

void RecordArmAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    switch(_step) {
    	case(1): {
            Track *track = dynamic_cast<Track*>(ctx.project->getUnitById(_action.targetId));
            if(!track) {
                LOG_ERROR("failed to convert to Track ptr %u", _action.targetId);
                abortAction();
                return;
            }

            bool state = floatToBool(_action.recordState);
            LOG_INFO("Track id: %u record arm toggled, new state %s, source %s", 
            _action.targetId, 
            (state ? "On" : "Off"), 
            (_action.recordSource == RecordSource::Audio ? "Audio" : "Midi"));
            
            if(state) {
                LOG_INFO("Preparing to record on track id: %u", _action.targetId);
        
                AudioDriver * driver = ctx.engine->driver();
                int numInputs = driver->inputCount();
                int numOutputs = driver->outputCount();

                slr::frame_t inputLatency = 0;
                for(int i=0; i<numInputs; ++i) {
                    inputLatency = std::max(inputLatency, driver->inputLatency(i));
                }
                slr::frame_t outputLatency = 0;
                for(int i=0; i<numOutputs; ++i) {
                    outputLatency = std::max(outputLatency, driver->outputLatency(i));
                }

                slr::frame_t latencyToCompensate = inputLatency+outputLatency+SettingsManager::getManualLatencyCompensation();
                LOG_INFO("Total latency compensation for recording is %lu", latencyToCompensate);
                if(_action.recordSource == RecordSource::Audio) {
                    if(!track->prepareAudioRecord(ctx.fileWorker, latencyToCompensate)) {
                        LOG_ERROR("Failed to prepare for audio record");
                        return;
                    }
                } else {
                    if(!track->prepareMidiRecord(ctx.fileWorker)) {
                        LOG_ERROR("Failed to prepare for midi record");
                        return;
                    }
                }
            }

            _flat.track = track;
            _flat.recordState = _action.recordState;
            _flat.recordSource = _action.recordSource;
            _flat.completed.store(false);
            _task = makeRtTask(&_flat);
            
            setState(ActionState::Waiting);
            ctx.EmitRtTask(&_task);
    	} break;
    	case(2): {
            bool state = floatToBool(_flat.recordState);
            LOG_INFO("Track id: %u, record arm switched, new state: %s", _flat.track->id(), (state ? "On" : "Off"));

            if(!state) { //if disabling - release
                LOG_INFO("Releasing record target for track id: %u", _flat.track->id());
                _flat.track->releaseRecordTarget(ctx.fileWorker);
            }

            AudioUnitView *uview = ctx.projectView->getUnitById(_flat.track->id());
            TrackView *trview = dynamic_cast<TrackView*>(uview);

            if(trview == nullptr) {
                LOG_ERROR("Failed to find Track View");
                return;
            }

            trview->update();

            setState(ActionState::Finished);
    	} break;
        default: assert(false && "Unreachable"); break;
    }
}

void RecordArmAction::checkWaitingCondition(ControlContext &ctx) {
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


void RecordArmAction::RecordArmFlat::execRT() {
    bool record = floatToBool(recordState);
    track->setRecord(record);
    track->setRecordSource(recordSource);
    completed.store(true, std::memory_order_release);
}

std::unique_ptr<ActionExecutable> createRecordArmAction(const ActionBase *base) {
    return std::make_unique<RecordArmAction>(base);
}

}