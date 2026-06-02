// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "units/Track/Actions/ReinitTrackRecord.h"

#include "core/primitives/ActionBase.h"
#include "core/primitives/ControlContext.h"

#include "core/drivers/AudioDriver.h"
#include "core/Project.h"
#include "core/SettingsManager.h"
#include "core/RtEngine.h"
#include "units/Track/Track.h"

#include "logger.h"

#include <cassert>

namespace slr {

ReinitTrackRecordAction::ReinitTrackRecordAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::ReinitTrackRecord*>(base)) )
{

}

ReinitTrackRecordAction::~ReinitTrackRecordAction() {

}

void ReinitTrackRecordAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    switch(_step) {
    	case(1): {
            bool success = true;
            Track * track = _action.track;
            RecordSource src = track->recordSource();

            
            LOG_INFO("Reinitializing record target for track id: %u, source %s", 
                        track->id(),
                        (src == RecordSource::Audio ? "Audio" : "Midi"));
            
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
            if(src == RecordSource::Audio) {
                if(!track->prepareAudioRecord(ctx.fileWorker, latencyToCompensate)) {
                    success = false;
                }
            } else {
                if(!track->prepareMidiRecord(ctx.fileWorker)) {
                    success = false;
                }
            }

            if(!success) {
                LOG_ERROR("Failed to reinit track id:%u for record", track->id());
                abortAction();
                return;
            }
            
            _flat.track = track;
            _flat.completed.store(false);
            _task = makeRtTask(&_flat);
            
            setState(ActionState::Waiting);
            ctx.EmitRtTask(&_task);
    	} break;
    	case(2): {
            
            LOG_INFO("Record reinit done");
            setState(ActionState::Finished);
    	} break;
        default: assert(false && "Unreachable"); break;
    }
}

void ReinitTrackRecordAction::checkWaitingCondition(ControlContext &ctx) {
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


void ReinitTrackRecordAction::ReinitFlat::execRT() {
    track->setRecord(true);
    completed.store(true, std::memory_order_release);
}

std::unique_ptr<ActionExecutable> createReinitTrackRecordAction(const ActionBase *base) {
    return std::make_unique<ReinitTrackRecordAction>(base);
}

}