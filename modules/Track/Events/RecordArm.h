// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK RecordArm 

EV
INCLUDE "defines.h"
struct RecordArm {
    ID targetId;
    float recordState; //on off
    RecordSource recordSource; //audio midi 0.0 - Audio, 1.0 - midi
};

FLAT_REQ 
INCLUDE "defines.h"
struct RecordArm {
    Track * track; //-> class Track;
    float recordState;
    RecordSource recordSource;
};

EV_HANDLE
INCLUDE "core/ControlEngine.h"
INCLUDE "core/FlatEvents.h"
INCLUDE "core/Project.h"
INCLUDE "modules/Track/Track.h"
INCLUDE "core/primitives/AudioUnit.h"
INCLUDE "core/RtEngine.h"
INCLUDE "core/drivers/AudioDriver.h"
INCLUDE "core/utility/helper.h"
INCLUDE "defines.h"
INCLUDE "core/SettingsManager.h"
INCLUDE <cmath>
void handleEvent(const ControlContext &ctx, const Events::RecordArm &e) {
    AudioUnit * u = ctx.project->getUnitById(e.targetId);
    Track * trg = dynamic_cast<Track*>(u);
    if(!trg) {
        LOG_ERROR("Failed to convert to Track ptr");
        return;
    }

    bool state = floatToBool(e.recordState);
    LOG_INFO("Track id: %u record arm toggled, new state %s, source %s", 
                e.targetId, 
                (state ? "On" : "Off"), 
                (e.recordSource == RecordSource::Audio ? "Audio" : "Midi"));
    
    if(state) { //if enabling - prepare
        LOG_INFO("Preparing to record on track id: %u", e.targetId);
        
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
        if(e.recordSource == RecordSource::Audio) {
            if(!trg->prepareAudioRecord(ctx.fileWorker, latencyToCompensate)) {
                LOG_ERROR("Failed to prepare for audio record");
                return;
            }
        } else {
            if(!trg->prepareMidiRecord(ctx.fileWorker)) {
                LOG_ERROR("Failed to prepare for midi record");
                return;
            }
        }
    }

    FlatEvents::FlatControl rec;
    rec.type = FlatEvents::FlatControl::Type::RecordArm;
    rec.commandId = ControlEngine::generateCommandId();
    rec.recordArm.track = trg;
    rec.recordArm.recordState = e.recordState;
    rec.recordArm.recordSource = e.recordSource;
    ControlEngine::emitRtControl(rec);
}
END_HANDLE

RT_HANDLE 
INCLUDE "modules/Track/Track.h"
&Track::setRecordArm
END_HANDLE

RESP_HANDLE
INCLUDE "ui/uiControls.h"
INCLUDE "snapshots/ProjectView.h"
INCLUDE "modules/Track/TrackView.h"
INCLUDE "snapshots/AudioUnitView.h"
INCLUDE "core/utility/helper.h"
void handleRecordArmResponse(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
    if(resp.status == Status::Ok) {
        bool state = floatToBool(resp.recordArm.recordState);
        LOG_INFO("Track id: %u, record arm switched, new state: %s", resp.recordArm.track->id(), (state ? "On" : "Off"));

        if(!state) { //if disabling - release
            LOG_INFO("Releasing record target for track id: %u", resp.recordArm.track->id());
            resp.recordArm.track->releaseRecordTarget(ctx.fileWorker);
        }

        AudioUnitView * uview = ctx.projectView->getUnitById(resp.recordArm.track->id());
        TrackView * trview = static_cast<TrackView*>(uview);

        if(trview == nullptr) {
            LOG_ERROR("Failed to find Track View");
            return;
        }

        trview->update();
        UIControls::updateModuleUI(resp.recordArm.track->id());
    } else {
        //user notify...
        LOG_ERROR("Failed to update Record state of track id: %u. Record: %s, type: %s", 
                    resp.recordArm.track->id(), 
                    (floatToBool(resp.recordArm.recordState) ? "On" : "Off"), 
                    (resp.recordArm.recordSource == RecordSource::Audio ? "Audio" : "Midi"));
    }
}
END_HANDLE

END_BLOCK
