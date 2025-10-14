// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK

EV
INCLUDE "defines.h"
struct ModContainerItem {
    ID unitId;
    ID itemId;
    frame_t startPosition;
    frame_t length;
    bool muted;
};

FLAT_REQ
INCLUDE "defines.h"
struct ModContainerItem {
    AudioUnit * unit; //-> class AudioUnit;
    ContainerItem * item; //->class ContainerItem;
    frame_t startPosition;
    frame_t length;
    bool muted;
};

EV_HANDLE
INCLUDE "core/Project.h"
INCLUDE "core/primitives/AudioUnit.h"
INCLUDE "core/primitives/FileContainer.h"
INCLUDE "core/FlatEvents.h"
INCLUDE "core/ControlEngine.h"
INCLUDE "logger.h"
void handleEvent(const ControlContext &ctx, const Events::ModContainerItem &e) {
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

    const std::vector<ContainerItem*> *items = unit->items();
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
    ctl.modContainerItem.unit = unit;
    ctl.modContainerItem.item = item;
    ctl.modContainerItem.startPosition = e.startPosition;
    ctl.modContainerItem.length = e.length;
    ctl.modContainerItem.muted = e.muted;
    ControlEngine::emitRtControl(ctl);
}
END_HANDLE

RT_HANDLE
INCLUDE "core/primitives/AudioUnit.h"
&AudioUnit::modifyContainerItem
END_HANDLE

RESP_HANDLE
INCLUDE "snapshots/ProjectView.h"
INCLUDE "snapshots/FileContainerView.h"
INCLUDE "core/primitives/FileContainer.h"
INCLUDE "ui/uiControls.h"
INCLUDE "Status.h"
INCLUDE "logger.h"
void handleModifyContainerItemResponse(const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
    if(resp.status == Status::NotOk) {
        LOG_ERROR("Failed to modify container item:  unit %u, item %u, start %lu, length %lu, muted %s",
                    resp.modContainerItem.unit->id(),
                    resp.modContainerItem.item->_uniqueId,
                    resp.modContainerItem.startPosition,
                    resp.modContainerItem.length,
                    (resp.modContainerItem.muted ? "true" : "false"));
        return;
    }

    LOG_INFO("Container item modified:  unit %u, item %u, start %lu, length %lu, muted %s",
                    resp.modContainerItem.unit->id(),
                    resp.modContainerItem.item->_uniqueId,
                    resp.modContainerItem.startPosition,
                    resp.modContainerItem.length,
                    (resp.modContainerItem.muted ? "true" : "false"));

    AudioUnitView * uview = ctx.projectView->getUnitById(resp.modContainerItem.unit->id());
    if(!uview) {
        LOG_ERROR("Failed to find AudioUnitView for unit %u", resp.modContainerItem.unit->id());
        return;
    }

    std::vector<ContainerItemView*> items = uview->_fileList._items;
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

    UIControls::updateModuleUI(uview->id());
}
END_HANDLE

END_BLOCK