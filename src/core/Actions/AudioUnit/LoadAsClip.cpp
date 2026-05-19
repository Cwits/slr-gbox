// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Actions/AudioUnit/LoadAsClip.h"
#include "core/primitives/ActionBase.h"
#include "core/primitives/ControlContext.h"

#include "core/primitives/AudioUnit.h"
#include "core/primitives/FileContainer.h"
#include "core/Project.h"
#include "core/FileWorker.h"
#include "core/FileTasks.h"

#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"
#include "snapshots/FileContainerView.h"

#include "logger.h"

namespace slr {


LoadAsClipAction::LoadAsClipAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::LoadAsClip*>(base))),
    _file(nullptr),
    _clip(nullptr)
{
}

LoadAsClipAction::~LoadAsClipAction() {

}

void LoadAsClipAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    switch(_step) {
        case(1): {
            if(std::holds_alternative<std::string>(_action.data)) {
                auto task = std::make_unique<Tasks::openFile>();
                task->path = std::get<std::string>(_action.data);
                if(_action.fileForcedId) task->forcedId = _action.fileForcedId.value();
                task->finished = [opened = &_opened](const File *file, bool success) {
                    opened->file = file;
                    opened->result = success;
                    opened->completed.store(true, std::memory_order_release);
                };
                ctx.fileWorker->addTask(std::move(task));
                setState(ActionState::Waiting);
            } else if(std::holds_alternative<File*>(_action.data)) {
                _file = std::get<File*>(_action.data);

                // if(!isFileValid(_file)) {
                //     LOG_ERROR("Invalid file");
                //     abortAction();
                //     return;
                // }

                _step = 2;
            } else if(std::holds_alternative<ClipItem*>(_action.data)) {
                _clip = std::get<ClipItem*>(_action.data);

                // if(!isClipValid(_clip)) {
                //     LOG_ERROR("Invalid Clip");
                //     abortAction();
                //     return;
                // }

                _step = 3;
            }
        } break;
        case(2): {
            //file loaded, make it to clip
            if(!_file) {
                LOG_ERROR("Failed action");
                abortAction();
                return;
            }

            if(_action.clipForcedId)
                _clip = ctx.project->clipStorage().newClip(
                    _file, 
                    _action.startOffset, 
                    _action.clipForcedId.value()
                );
            else 
                _clip = ctx.project->clipStorage().newClip(
                    _file, 
                    _action.startOffset
                );
            
            _step = 3;
        } break;
        case(3): {
            if(!_clip) {
                LOG_ERROR("Clip is empty");
                abortAction();
                return;
            }

            ClipContainerBuffer &unitStorage = ctx.project->getClipContainerBufferById(_action.targetId);

            ClipContainer *modContainer = unitStorage.modifiableContainer();
            const ClipContainer *inUseContainer = unitStorage.inUseContainer();

            if(inUseContainer->size()+1 > inUseContainer->capacity()) {
                if(ctx.prohibitAllocation(inUseContainer->capacity()*2*sizeof(ClipItem*))) {
                    LOG_WARN("Low on memory, prohibit creating new things");
                    return;
                }
                modContainer->clear();
                modContainer->reserve(inUseContainer->capacity()*2);
                *modContainer = *(inUseContainer);
            } else {
                modContainer->clear();
                *modContainer = *(inUseContainer);
            }

            if(ctx.prohibitAllocation(sizeof(ClipItem))) {
                LOG_WARN("Low on memory, prohibit creating new things");
                return;
            }
        
            modContainer->push_back(_clip);

            AudioUnit *unit = ctx.project->getUnitById(_action.targetId);
            if(!unit) {
                LOG_ERROR("No unit with such id %u", _action.targetId);
                abortAction();
                return;
            }

            _flat.target = unit;
            _flat.container = modContainer;
            _flat.completed.store(false);

            _task = makeRtTask(&_flat);
            setState(ActionState::Waiting);

            ctx.EmitRtTask(&_task);
        } break;
        case(4): {
            ClipContainerBuffer &unitStorage = ctx.project->getClipContainerBufferById(_action.targetId);
            unitStorage.containerSwapped();

            AudioUnitView * uview = ctx.projectView->getUnitById(_action.targetId);
            if(!uview) {
                LOG_ERROR("Failed to find AudioUnitView with id %u", _action.targetId);
                abortAction();
                return;
            }

            if(ctx.prohibitAllocation(sizeof(ClipItemView))) {
                LOG_WARN("Low on memory");
                //need some cleanup
                abortAction(); 
                return;
            }

            ClipItemView *cview = ctx.projectView->createClipView(_clip);
            if(!cview) {
                LOG_WARN("Failed to create ClipView");
                //need some cleanup
                abortAction();
                return;
            }

            uview->_clipContainer.addClipItem(cview);
            
            markDelete();
            setState(ActionState::Finished);
        } break;
        default: assert(false && "Unreachable"); break;
    }
}

void LoadAsClipAction::checkWaitingCondition(ControlContext &ctx) {
    assert(getState() == ActionState::Waiting);

    switch(_step) {
        case(1): {
            bool res = _opened.completed.load(std::memory_order_acquire);
            if(res) {
                if(!_opened.result) {
                    LOG_ERROR("Failed to open a file");
                    abortAction();
                    return;
                }

                _file = _opened.file;
                _step = 2;
                setState(ActionState::Executing);
            }
        } break;
        case(3): {
            bool res = _flat.completed.load(std::memory_order_acquire);
            if(res) {
                _step = 4;
                setState(ActionState::Executing);
            }
        } break;        
        default: assert(false && "Unreachable"); break;
    }
}

void LoadAsClipAction::undo(ControlContext &ctx) {

}

void LoadAsClipAction::redo(ControlContext &ctx) {

}


std::unique_ptr<ActionExecutable> createLoadAsClipAction(const ActionBase*base) {
    return std::make_unique<LoadAsClipAction>(base);
}

}
