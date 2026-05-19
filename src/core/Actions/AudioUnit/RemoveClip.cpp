// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Actions/AudioUnit/RemoveClip.h"

#include "core/primitives/ActionBase.h"
#include "core/primitives/ControlContext.h"

#include "core/Project.h"

#include "core/primitives/AudioUnit.h"
#include "core/primitives/FileContainer.h"

#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"
#include "snapshots/FileContainerView.h"

#include "ui/uiControls.h"

#include "logger.h"

#include <cassert>

namespace slr {

RemoveClipAction::RemoveClipAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::RemoveClip*>(base)) )
{

}

RemoveClipAction::~RemoveClipAction() {

}

void RemoveClipAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    switch(_step) {
    	case(1): {
            
            AudioUnitView * uview = ctx.projectView->getUnitById(_action.targetId);
            if(!uview) {
                LOG_ERROR("Failed to find AudioUnitView for track: %u", _action.targetId);
                abortAction();
                return;
            }
            uview->_clipContainer.deleteClipViewItem(_action.clipId);

            //UIControls::removeClipFromUnit(_action.targetId, _action.clipId);
            //assume at this point that ui was deleted and there be no actions to this clip
            //but it must be somehow different

            AudioUnit * unit = ctx.project->getUnitById(_action.targetId);
            if(!unit) {
                LOG_ERROR("Error finding unit");
                abortAction();
                return;
            }

            const ClipItem *item = nullptr;
            const ClipContainer * clips = unit->clips();
            auto it = std::find_if(
                clips->begin(), 
                clips->end(), 
                [clipid = _action.clipId](const ClipItem *i) {
                return i->id() == clipid;
            });

            if(it != clips->end()) item = *it;

            if(item == nullptr) {
                LOG_ERROR("Failed to find file with id %u in unit %u", _action.clipId, _action.targetId);
                return;
            }

            ClipContainerBuffer &storage = ctx.project->getClipContainerBufferById(_action.targetId);
            ClipContainer *modifiable = storage.modifiableContainer();

            modifiable->clear();
            if(modifiable->capacity() < clips->capacity()) {
                modifiable->resize(clips->capacity());
            }

            *modifiable = *clips;

            modifiable->erase(
                std::remove_if(
                    modifiable->begin(),
                    modifiable->end(),
                    [item](const ClipItem *i) {
                        return i == item;
                    }
            ), modifiable->end());

            _flat.target = unit;
            _flat.container = modifiable;
            _flat.completed.store(false);
            _task = makeRtTask(&_flat);

            setState(ActionState::Waiting);
            ctx.EmitRtTask(&_task);
    	} break;
    	case(2): {
            //huh?
            /* 
                not deleting Clip and closing file totally.
                May add some reference counter to file and some kind of clean up process to close and free such memory.
                otherwise they stay in memory so user can reuse them quickly(need some kind of access procedure for it)
            */
            // const ID & unitId = resp.swapContainer.unit->id();

            //remove from memory owner
            // ClipContainerMap &map = ctx.project->clipContainerMap();
            // ClipStorage & clipStorage = map.at(unitId);
            // ClipContainerBuffer &clipStorage = ctx.project->getClipContainerBufferById(unitId);

            // std::vector<std::unique_ptr<ClipItem>> &owner = clipStorage._clipsOwner;
            // owner.erase(std::remove_if(owner.begin(), owner.end(), [itemId](const std::unique_ptr<ClipItem> & itm) {
            //     return itemId == itm->_uniqueId;
            // }), owner.end());

            LOG_INFO("Clip %u removed from unit %u successfully", _action.clipId, _action.targetId);
        
            markDelete();
            setState(ActionState::Finished);
    	} break;
        default: assert(false && "Unreachable"); break;
    }
}

void RemoveClipAction::checkWaitingCondition(ControlContext &ctx) {
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

void RemoveClipAction::undo(ControlContext &ctx) {

}

void RemoveClipAction::redo(ControlContext &ctx) {

}

std::unique_ptr<ActionExecutable> createRemoveClipAction(const ActionBase *base) {
    return std::make_unique<RemoveClipAction>(base);
}

}