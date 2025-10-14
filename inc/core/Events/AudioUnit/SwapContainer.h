// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK SwapContainer

FLAT_REQ
INCLUDE <vector>
struct SwapContainer {
    AudioUnit * unit; //-> class AudioUnit;
    std::vector<ContainerItem*> * container; //-> class ContainerItem;
};

FLAT_RESP
INCLUDE <vector>
struct SwapContainer {
    AudioUnit * unit;
    std::vector<ContainerItem*> * oldContainer; //-> class ContainerItem;
    std::vector<ContainerItem*> * newContainer;
};
 
RT_HANDLE
INCLUDE "core/primitives/AudioUnit.h"
&AudioUnit::swapContainer
END_HANDLE

RESP_HANDLE
INCLUDE "core/Project.h"
INCLUDE "logger.h"
void handleContainerSwapped(const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
    if(resp.status == Status::Ok) {
        LOG_INFO("Container for track %u swapped, deleting old one", resp.swapContainer.unit->id());
        delete resp.swapContainer.oldContainer;
    } else {
        LOG_ERROR("Failed to swap Track container");
    }
}
END_HANDLE

END_BLOCK