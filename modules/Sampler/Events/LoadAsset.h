// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK SetParameter

EV
INCLUDE "defines.h"
INCLUDE <string>
struct LoadAsset {
    std::string path;
    ID unitId;
};

EV
INCLUDE "defines.h"
struct AssetOpened {
    const AudioFile * file; //-> class AudioFile;
    ID unitId;
};

FLAT_REQ
struct AssetOpened {
    Sampler * sampler; //-> class Sampler;
    const AudioFile * file;
};

EV_HANDLE
INCLUDE "core/FileTasks.h"
INCLUDE "core/FileWorker.h"
INCLUDE "core/primitives/AudioFile.h"
INCLUDE "logger.h"
INCLUDE <memory>
INCLUDE <string>
void handleEvent(const ControlContext &ctx, const Events::LoadAsset &e) {
    LOG_INFO("Loading asset for sampler");
	if(ctx.prohibitAllocation(0)) {
		LOG_WARN("Low on memory, prohibit creating new things");
		return;
    }

    auto task = std::make_unique<Tasks::openFile>();
	task->path = e.path;
	task->targetId = e.unitId;
	task->fileStartPosition = 0;
	task->finished = [](bool success, const ID targetId, const File * file, const std::string & path, const frame_t fileStartPosition) {
		if(!success) {
			LOG_ERROR("File Worker failed to load file %s", path.c_str());
			return;
		}

		Events::AssetOpened e = {
            .file = dynamic_cast<const AudioFile*>(file),
            .unitId = targetId
        };

        EmitEvent(e);
	};
	ctx.fileWorker->addTask(std::move(task));
}
END_HANDLE

EV_HANDLE
INCLUDE "modules/Sampler/Sampler.h"
void handleEvent(const ControlContext &ctx, const Events::AssetOpened &e) {
    AudioUnit *unit = ctx.project->getUnitById(e.unitId);
    if(!unit) {
        LOG_ERROR("Failed to find unit %u", e.unitId);
        return;
    }

    Sampler * sampler = dynamic_cast<Sampler*>(unit);
    if(!sampler) {
        LOG_ERROR("Failed to convert unit to sampler");
        return;
    }

    FlatEvents::FlatControl ctl;
    ctl.type = FlatEvents::FlatControl::Type::AssetOpened;
    ctl.assetOpened.sampler = sampler;
    ctl.assetOpened.file = e.file;
    ControlEngine::emitRtControl(ctl);
}
END_HANDLE

RT_HANDLE
INCLUDE "modules/Sampler/Sampler.h"
&Sampler::assetOpened
END_HANDLE

RESP_HANDLE
INCLUDE "snapshots/ProjectView.h"
INCLUDE "modules/Sampler/SamplerView.h"
INCLUDE "logger.h"
INCLUDE "common/Status.h"
void handleAssetOpenedResponse(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
    //update UI
    if(resp.status != Common::Status::Ok) {
        LOG_ERROR("Something failed");
        return;
    }

    AudioUnitView * view = ctx.projectView->getUnitById(resp.assetOpened.sampler->id());
    if(!view) {
        LOG_ERROR("Failed to find view");
        return;
    }

    SamplerView * sview = dynamic_cast<SamplerView*>(view);
    if(!sview) {
        LOG_ERROR("That view wasn't samplers");
        return;
    }

    sview->update();

}
END_HANDLE

END_BLOCK