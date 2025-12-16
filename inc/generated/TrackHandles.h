/* This file is generated automatically, do not edit manually */
#pragma once
#include "core/ControlEngine.h"
#include "core/primitives/File.h"
#include "ui/uiControls.h"
#include "core/utility/helper.h"
#include "core/primitives/AudioBuffer.h"
#include "logger.h"
#include "core/Project.h"
#include "snapshots/AudioUnitView.h"
#include "logger.h"  
#include "core/FileWorker.h"
#include "core/FlatEvents.h"
#include "core/primitives/AudioFile.h"
#include <string>
#include "snapshots/ProjectView.h"
#include "core/drivers/AudioDriver.h"
#include "defines.h"
#include "modules/Track/Track.h"
#include <memory>
#include "core/utility/basicAudioManipulation.h"
#include "core/SettingsManager.h"
#include "Status.h"
#include "core/RtEngine.h"
#include "core/primitives/AudioUnit.h"
#include "core/FileTasks.h"
#include "modules/Track/TrackView.h"
#include <cmath>
#include "snapshots/FileContainerView.h"
#include "core/primitives/FileContainer.h"
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

inline void handleAppendItemResponse(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
    if(resp.status == Status::Ok) {
		LOG_INFO("Item %u appended successfully on track id: %u", resp.appendItem.item->_uniqueId, resp.appendItem.track->id());
		// ProjectView * psnap = ControlEngine::getLastSnapshot();
		
		AudioUnitView * uview = ctx.projectView->getUnitById(resp.appendItem.track->id());
		TrackView * tv = static_cast<TrackView*>(uview);
		if(!tv) {
			LOG_ERROR("Failed to find TrackView with id %u", resp.appendItem.track->id());
			return;
		}

		ContainerItemView * item = new ContainerItemView(resp.appendItem.item);
		tv->_fileList._items.push_back(item);
		// ctx.projectView->getTrack(resp.appendItem.track->id())->_fileList._items.push_back(item);
		
		// UIControls::addFileToTrackUI(tv->id(), tv->_fileList._items.back());
		UIControls::updateModuleUI(tv->id());
		// swapSnapshot();
	} else { 
		LOG_ERROR("Need to handle error!");
	}
}

inline void handleContainerSwapped(const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
    if(resp.status == Status::Ok) {
        LOG_INFO("Container for track %u swapped, deleting old one", resp.swapContainer.track->id());
        delete resp.swapContainer.oldContainer;
    } else {
        LOG_ERROR("Failed to swap Track container");
    }
}

inline void handleModifyContainerItemResponse(const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
    if(resp.status == Status::NotOk) {
        LOG_ERROR("Failed to modify container item:  unit %u, item %u, start %lu, length %lu, muted %s",
                    resp.modContainerItem.track->id(),
                    resp.modContainerItem.item->_uniqueId,
                    resp.modContainerItem.startPosition,
                    resp.modContainerItem.length,
                    (resp.modContainerItem.muted ? "true" : "false"));
        return;
    }

    LOG_INFO("Container item modified:  unit %u, item %u, start %lu, length %lu, muted %s",
                    resp.modContainerItem.track->id(),
                    resp.modContainerItem.item->_uniqueId,
                    resp.modContainerItem.startPosition,
                    resp.modContainerItem.length,
                    (resp.modContainerItem.muted ? "true" : "false"));

    AudioUnitView * uview = ctx.projectView->getUnitById(resp.modContainerItem.track->id());
    if(!uview) {
        LOG_ERROR("Failed to find AudioUnitView for unit %u", resp.modContainerItem.track->id());
        return;  
    }
    TrackView * tv = static_cast<TrackView*>(uview);
	if(!tv) {
		LOG_ERROR("Failed to find TrackView with id %u", resp.appendItem.track->id());
		return;
	}

    std::vector<ContainerItemView*> items = tv->_fileList._items;
    ContainerItemView * item = nullptr;
    for(std::size_t i=0; i<items.size(); ++i) {
        if(resp.modContainerItem.item->_uniqueId == items.at(i)->_uniqueId) {
            item = items.at(i);
            break;
        }
    }

    if(!item) {
        LOG_ERROR("Failed to find ContainerItemView for item %u", resp.modContainerItem.item->_uniqueId);
        return;
    }

    item->_startPosition = resp.modContainerItem.item->_startPosition;
    item->_length = resp.modContainerItem.item->_length;
    item->_muted = resp.modContainerItem.item->_muted;

    UIControls::updateModuleUI(tv->id());
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

inline void handleEvent(const ControlContext &ctx, const Events::OpenFile &e) {
	LOG_INFO("Opening a file %s", e.path.c_str());


	AudioUnit * u = ctx.project->getUnitById(e.targetId);
	if(!u) {
		LOG_ERROR("No unit with such id %u", e.targetId);
		return;
	}
	Track * trg = dynamic_cast<Track*>(u);
	if(!trg) {
		LOG_ERROR("Failed to convert to Track ptr");
		return;
	}

    auto task = std::make_unique<Tasks::openFile>();
	task->path = e.path;
	task->targetId = e.targetId;
	ctx.fileWorker->addTask(std::move(task));
}

inline void handleEvent(const ControlContext &ctx, const Events::FileOpened &e) {
    if(e.status == Status::Ok) {
		LOG_INFO("File %s opened with id %u, adding to target track %u", e.file->path().c_str(), e.file->id(), e.targetId);

		ContainerItem * item = new ContainerItem(e.file);
		// item->_file = e.file;
		item->_startPosition = 0;
		item->_length = e.file->frames();
		item->_muted = false;
		// item->_uniqueId = e.file->id();

		AudioUnit * u = ctx.project->getUnitById(e.targetId);
		if(!u) {
			LOG_ERROR("No unit with such id %u", e.targetId);
			return;
		}
		Track * trg = static_cast<Track*>(u);
		if(!trg) {
			LOG_ERROR("Failed to convert to Track ptr");
			return;
		}

		std::size_t size = trg->items()->size();
		if(trg->items()->size()+1 > trg->items()->capacity()) {
			//create new vector
			LOG_INFO("Track id:%u container run out of space, resizing", e.targetId);
			std::vector<ContainerItem*> * container = new std::vector<ContainerItem*>();
			container->reserve(size*2);
			*container = *trg->items();

			container->push_back(item);

			FlatEvents::FlatControl ctrl;
			ctrl.type = FlatEvents::FlatControl::Type::SwapContainer;
			ctrl.commandId = ControlEngine::generateCommandId();
			ctrl.swapContainer.track = trg;
			ctrl.swapContainer.container = container;
   
			ControlEngine::awaitRtResult(ctrl, [item](const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
				FlatEvents::FlatResponse fake;
				fake.type = FlatEvents::FlatResponse::Type::AppendItem;
				fake.status = resp.status;
				fake.appendItem.track = resp.swapContainer.track;
				fake.appendItem.item = item;
				handleAppendItemResponse(ctx, fake);
			});
		} else {
			//just append
			LOG_INFO("Appending item %u to track id: %u", item->_uniqueId, e.targetId);
			FlatEvents::FlatControl ctrl;
			ctrl.type = FlatEvents::FlatControl::Type::AppendItem;
			ctrl.commandId = ControlEngine::generateCommandId();
			ctrl.appendItem.track = trg;
			ctrl.appendItem.item = item;
			ControlEngine::emitRtControl(ctrl);
		}
	} else {
        //TODO: failed to open
		LOG_ERROR("Failed to open audio file");
    }
}

inline void handleEvent(const ControlContext &ctx, const Events::RemoveFile &e) {    
    LOG_INFO("Removing file UI %u from track %u", e.fileId, e.trackId);
    // ProjectView * snap = ctx.projectView;
    // TrackView * trview = snap->getTrack(e.trackId);
    AudioUnitView * uview = ctx.projectView->getUnitById(e.trackId);
    TrackView * trview = static_cast<TrackView*>(uview);
    if(trview == nullptr) {
        LOG_ERROR("Failed to find TrackView for track: %u", e.trackId);
        return;
    }
    //need to remove containerview as well
    //find ContainerItemView
    std::size_t idxViews = 0;
    ContainerItemView * itemView = nullptr;
    std::vector<ContainerItemView*> & itemsViews = trview->_fileList._items;
    for(auto & itm : itemsViews) {
        if(itm->_uniqueId == e.fileId) {   
            itemView = itm;
            break;
        }
        idxViews++;
    } 

    if(itemView == nullptr) {
        LOG_ERROR("failed to find FileView for file %u in track %u", e.fileId, e.trackId);
        return;
    }

    //remove
    // UIControls::removeFileFromTrackUI(trview, e.fileId);
    itemsViews.erase(itemsViews.begin()+idxViews);
    delete itemView;
    
    UIControls::updateModuleUI(e.trackId);
}

inline void handleEvent(const ControlContext &ctx, const Events::FileUIRemoved &e) { 
    LOG_INFO("Removing file id %u from track id %u", e.fileId, e.trackId);
    //remove
    AudioUnit * unit = ctx.project->getUnitById(e.trackId);
    Track * tr = dynamic_cast<Track*>(unit);
    if(tr == nullptr) {
        LOG_ERROR("Failed to find track, id: %u is wrong or track doesn't exists", e.trackId);
        return;
    }

    ContainerItem * item = nullptr;
    const std::vector<ContainerItem*> * items = tr->items();
    std::size_t idx = 0;
    for(std::size_t i=0; i<items->size(); ++i) {
        if(items->at(i)->_file->id() == e.fileId) {
            item = items->at(i);
            idx = i;
            break;
        }
    }

    if(item == nullptr) {
        LOG_ERROR("Failed to find file with id %u in track %u", e.fileId, e.trackId);
        return;
    }

    //prepare swapable container
    std::vector<ContainerItem*> * container = new std::vector<ContainerItem*>();
    *container = *items;
    //TODO: must check that item removed from container
    container->erase(container->begin()+idx);

    FlatEvents::FlatControl swap;
    swap.type = FlatEvents::FlatControl::Type::SwapContainer;
    swap.commandId = ControlEngine::generateCommandId();
    swap.swapContainer.track = tr;
    swap.swapContainer.container = container;

    ControlEngine::awaitRtResult(swap, [](const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
        if(resp.status == Status::Ok) {
            LOG_INFO("Item removed from track %u successfully", resp.swapContainer.track->id());
        } else { 
            LOG_ERROR("Failed to remove item from track %u", resp.swapContainer.track->id());
        }
    });
}

inline void handleEvent(const ControlContext &ctx, const Events::ModContainerItem &e) {
    LOG_INFO("Modify Container Item Event: unit %u, item %u, start %lu, length %lu, muted %s",
                    e.unitId,
                    e.itemId,
                    e.startPosition,
                    e.length,
                    (e.muted ? "true" : "false"));
    
    AudioUnit * unit = ctx.project->getUnitById(e.unitId);
    if(!unit) {
        LOG_ERROR("Modify Container Item failed to find unit %u", e.unitId);
        return;
    }

    Track * track = static_cast<Track*>(unit);
    if(!track) {
        LOG_ERROR("Unit is not a track");
        return;
    }

    const std::vector<ContainerItem*> *items = track->items();
    ContainerItem * item = nullptr;
    for(std::size_t i=0; i<items->size(); ++i) {
        if(items->at(i)->_uniqueId == e.itemId) {
            item = items->at(i);
            break;
        }
    }

    if(!item) {
        LOG_ERROR("Modify Container Item failed to find item %u in unit %u", e.itemId, e.unitId);
        return;
    }

    FlatEvents::FlatControl ctl;
    ctl.type = FlatEvents::FlatControl::Type::ModContainerItem;
    ctl.commandId = ControlEngine::generateCommandId();
    ctl.modContainerItem.track = track;
    ctl.modContainerItem.item = item;
    ctl.modContainerItem.startPosition = e.startPosition;
    ctl.modContainerItem.length = e.length;
    ctl.modContainerItem.muted = e.muted;
    ControlEngine::emitRtControl(ctl);
}

} //namespace slr
