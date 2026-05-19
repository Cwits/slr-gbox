// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "units/Sampler/Actions/LoadAsset.h"

#include "units/Sampler/Sampler.h"

#include "core/primitives/ActionBase.h"
#include "core/primitives/AudioFile.h"
#include "core/primitives/ControlContext.h"
#include "core/FileWorker.h"
#include "core/FileTasks.h"
#include "core/utility/helper.h"

#include "core/Project.h"
#include "snapshots/ProjectView.h"
// #include "snapshots/AudioUnitView.h"
#include "units/Sampler/SamplerView.h"

#include "logger.h"

#include <cassert>

namespace slr {

LoadAssetAction::LoadAssetAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::LoadAsset*>(base)) )
{

}

LoadAssetAction::~LoadAssetAction() {

}

void LoadAssetAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    switch(_step) {
    	case(1): {
            if(std::holds_alternative<std::string>(_action.data)) {
                if(ctx.prohibitAllocation(0)) {
                    LOG_WARN("Low on memory, prohibit creating new things");
                    abortAction();
                    return;
                }

                const std::string &path = std::get<std::string>(_action.data);
                if(!pathHasExtention(Extention::Audio, path)) {
                    LOG_ERROR("Only audio files accepted %s", path.c_str());
                    abortAction();
                    return;
                }

                auto task = std::make_unique<Tasks::openFile>();
                task->path = std::get<std::string>(_action.data);
                task->finished = [opened = &_opened](const File *file, bool success) {
                    opened->file = file;
                    opened->result = success;
                    opened->completed.store(true, std::memory_order_release);
                };
                ctx.fileWorker->addTask(std::move(task));
                
                setState(ActionState::Waiting);
            } else if(std::holds_alternative<File*>(_action.data)) {
                ///...
                LOG_WARN("Unsupported yet");
                abortAction();
                return;
            } else if(std::holds_alternative<ClipItem*>(_action.data)) {
                LOG_WARN("Unsupported yet");
                abortAction();
                return;
            }
        } break;
        case(2): {
            AudioUnit *unit = ctx.project->getUnitById(_action.targetId);
            if(!unit) {
                LOG_ERROR("Failed to find unit %u", _action.targetId);
                return;
            }

            Sampler * sampler = dynamic_cast<Sampler*>(unit);
            if(!sampler) {
                LOG_ERROR("Failed to convert unit to sampler");
                return;
            }

            const AudioFile * afile = dynamic_cast<const AudioFile*>(_opened.file);
            if(!afile) {
                LOG_ERROR("Audio File required");
                abortAction();
                return;
            }

            _flat.sampler = sampler;
            _flat.file = afile;
            _flat.completed.store(false);
            _task = makeRtTask(&_flat);
            
            setState(ActionState::Waiting);
            ctx.EmitRtTask(&_task);
    	} break;
    	case(3): {
            AudioUnitView * view = ctx.projectView->getUnitById(_action.targetId);
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

            markDelete();
            setState(ActionState::Finished);
    	} break;
        default: assert(false && "Unreachable"); break;
    }
}

void LoadAssetAction::checkWaitingCondition(ControlContext &ctx) {
    assert(getState() == ActionState::Waiting);

    switch(_step) {
        case(1): {
            bool res = _opened.completed.load(std::memory_order_acquire);
            if(res) {
                if(!_opened.result) {
                    LOG_ERROR("Failed to open file");
                    abortAction();
                    return;
                }

                _step = 2;
                setState(ActionState::Executing);
            }
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


void LoadAssetAction::LoadAssetFlat::execRT() {
    sampler->clearAllVoices();
    sampler->asset(file);
    completed.store(true, std::memory_order_release);
}

std::unique_ptr<ActionExecutable> createLoadAssetAction(const ActionBase *base) {
    return std::make_unique<LoadAssetAction>(base);
}

}