// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK ReinitTrackRecord

FLAT_REQ
INCLUDE "Status.h"
struct ReinitTrackRecord {
    Status status;
    Track * track; //-> class Track;
};

RT_HANDLE
INCLUDE "modules/Track/Track.h"
&Track::reinitRecord
END_HANDLE

RESP_HANDLE
INCLUDE "core/ControlEngine.h"
INCLUDE "defines.h"
INCLUDE "core/SettingsManager.h"
void handleReinitTrackRecord(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
    bool success = true;
    Track * track = resp.reinitTrackRecord.track;
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
        return;
    }

    FlatEvents::FlatControl reinit;
    reinit.type = FlatEvents::FlatControl::Type::ReinitTrackRecord;
    reinit.commandId = ControlEngine::generateCommandId();
    reinit.reinitTrackRecord.track = track;
    reinit.reinitTrackRecord.status = Status::Ok;
    ControlEngine::emitRtControl(reinit);
    LOG_INFO("Record reinit done");
}
END_HANDLE

END_BLOCK