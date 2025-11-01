/* This file is generated automatically, do not edit manually */
#pragma once
#include "core/primitives/AudioUnit.h"
#include "core/ControlEngine.h"
#include "core/FileWorker.h"
#include "core/RtEngine.h"
#include "core/primitives/AudioBuffer.h"
#include "core/utility/basicAudioManipulation.h"
#include "logger.h"
#include "modules/Track/Track.h"
#include "snapshots/AudioUnitView.h"
#include <cmath>
#include "core/FlatEvents.h"
#include "core/primitives/AudioFile.h"
#include "core/drivers/AudioDriver.h"
#include "snapshots/ProjectView.h"
#include "modules/Track/TrackView.h"
#include <string>
#include "core/SettingsManager.h"
#include "core/primitives/File.h"
#include "defines.h"
#include "core/FileTasks.h"
#include "core/utility/helper.h"
#include "ui/uiControls.h"
#include "core/Project.h"
#include "core/primitives/ControlContext.h"

namespace slr {

inline void handleRecordArmResponse(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
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

inline void handleDumpRecordedAudio(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
    LOG_INFO("Dumping audio data to file %s", resp.dumpRecordedAudio.targetFile->name().c_str());
    auto task = std::make_unique<Tasks::dumpAudio>();
    task->buffer = resp.dumpRecordedAudio.targetBuffer;
    task->file = resp.dumpRecordedAudio.targetFile;
    task->size = resp.dumpRecordedAudio.size;

    task->callback = [rec = resp.dumpRecordedAudio.targetBuffer, 
                    target = resp.dumpRecordedAudio.targetFile, 
                    trackId = resp.dumpRecordedAudio.trackId](FileWorker * fw, bool success) {
        if(success) {
            LOG_INFO("audio data dumped successfully to file %s", target->name().c_str());
            AudioBuffer * buf = rec;
            
            for(int i=0; i<buf->channels(); ++i) {
                clearAudioBuffer((*buf)[i], buf->bufferSize());
            }
            
            AudioBufferManager::releaseRecord(buf);
            
            if(target->finalize()) {
                LOG_INFO("Finalizing audio file %s path %s", 
                    target->name().c_str(), 
                    target->path().c_str());
                // LOG_WARN("Finalizing audio file not implemented");
                //finalize file and return it to track
                std::string path = target->path();

                fw->closeTmpAudioFile(target);

                slr::Events::OpenFile e = {
                    .targetId = trackId,
                    .path = path
                };
                slr::EmitEvent(e);
            }
        } else {
            LOG_ERROR("Failed to dump audio data to file");
        }
    };

    ctx.fileWorker->addTask(std::move(task));
}

inline void handleReinitTrackRecord(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
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

inline void handleEvent(const ControlContext &ctx, const Events::RecordArm &e) {
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

} //namespace slr
