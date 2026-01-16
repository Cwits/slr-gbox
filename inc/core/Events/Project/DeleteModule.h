// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK DeleteModule

EV
INCLUDE "defines.h"
struct DeleteModule {
    ID targetId;
};

EV_HANDLE 
INCLUDE "core/ControlEngine.h"
INCLUDE "core/Project.h"
INCLUDE "ui/uiControls.h"
INCLUDE "snapshots/AudioUnitView.h"
INCLUDE "Status.h"
INCLUDE "logger.h"
void handleEvent(const ControlContext &ctx, const Events::DeleteModule &e) {
    //remove from ui
    UIControls::destroyModuleUI(e.targetId);

    ctx.project->removeRoutesForId(e.targetId);
    ctx.projectView->updateRoutes(ctx.project->routes());

    RenderPlan * newPlan = buildPlan(ctx.project);
    ctx.project->replaceEditablePlan(newPlan);
 
    FlatEvents::FlatControl ctl;
    ctl.type = FlatEvents::FlatControl::Type::SwapRenderPlan;
    ctl.swapRenderPlan.project = ctx.project;
    ctl.commandId = ControlEngine::generateCommandId();
    
    ControlEngine::awaitRtResult(ctl, [targetId = e.targetId](const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
        if(resp.status != Status::Ok) {
            LOG_ERROR("RT Engine failed to remove unit");
            return;
        }

        bool res = true;
        std::unique_ptr<AudioUnit> unit = ctx.project->removeUnit(targetId);
        if(unit == nullptr) {
            LOG_ERROR("Failed to find unit");
            res = false;
        }

        unit->destroy(ctx.bufferManager);
        // bool res = ctx.project->removeUnit(targetId);
        AudioUnitView * view = ctx.projectView->removeUnitView(targetId);
        
        if(view) {
            res &= true;
            delete view;
        } else {
            res &= false;
            LOG_ERROR("Failed to find view with id %u", targetId);
        }
        
        ClipContainerMap &map = ctx.project->clipContainerMap();
        auto it = map.find(targetId);
        if(it == map.end()) {
            LOG_ERROR("No clip storage for unit %u exists to delete", targetId);
            return;
        }
        
        map.erase(it);

        if(!res) {
            LOG_ERROR("Failed to delete unit from project");
        }
    });
}
END_HANDLE

END_BLOCK
