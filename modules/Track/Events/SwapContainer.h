START_BLOCK SwapContainer

FLAT_REQ
INCLUDE <vector>
struct SwapContainer {
    Track * track; //-> class Track;
    std::vector<ContainerItem*> * container; //-> class ContainerItem;
};

FLAT_RESP
INCLUDE <vector>
struct SwapContainer {
    Track * track;
    std::vector<ContainerItem*> * oldContainer; //-> class ContainerItem;
    std::vector<ContainerItem*> * newContainer;
};
 
RT_HANDLE
INCLUDE "modules/Track/Track.h"
&Track::swapContainer
END_HANDLE

RESP_HANDLE
INCLUDE "core/Project.h"
INCLUDE "logger.h"
void handleContainerSwapped(const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
    if(resp.status == Status::Ok) {
        LOG_INFO("Container for track %u swapped, deleting old one", resp.swapContainer.track->id());
        delete resp.swapContainer.oldContainer;
    } else {
        LOG_ERROR("Failed to swap Track container");
    }
}
END_HANDLE

END_BLOCK