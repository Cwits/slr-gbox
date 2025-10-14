// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK DeleteTrack

EV
INCLUDE "defines.h"
struct DeleteTrack {
    ID targetId;
};

EV_HANDLE 
INCLUDE "core/ControlEngine.h"
INCLUDE "core/Project.h"
INCLUDE "ui/uiControls.h"
INCLUDE "snapshots/AudioUnitView.h"
INCLUDE "Status.h"
INCLUDE "logger.h"
void handleEvent(const ControlContext &ctx, const Events::DeleteTrack &e) {
    // LOG_ERROR("Not supported");
    // return;
    //remove from ui
    UIControls::destroyModuleUI(e.targetId);

    ctx.project->removeRoutesForId(e.targetId);
    ctx.projectView->updateRoutes(ctx.project->routes());

    RenderPlan * newPlan = buildPlan(ctx.project, ctx.project->routes());
    ctx.project->replaceEditablePlan(newPlan);

    bool res = true;
    if(res) {
        //swap graph
        FlatEvents::FlatControl ctl;
        ctl.type = FlatEvents::FlatControl::Type::SwapRenderPlan;
        ctl.swapRenderPlan.project = ctx.project;
        ctl.commandId = ControlEngine::generateCommandId();
    
        ControlEngine::awaitRtResult(ctl, [targetId = e.targetId](const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
            if(resp.status == Status::Ok) {
                    bool res = ctx.project->removeUnit(targetId);
                    AudioUnitView * view = ctx.projectView->removeUnitView(targetId);

                    if(view) {
                        res &= true;
                        delete view;
                    } else {
                        res &= false;
                        LOG_ERROR("Failed to find view with id %u", targetId);
                    }
                    // res &= ctx.projectView->removeTrack(targetId);

                    if(!res) {
                        LOG_ERROR("Failed to delete track from project");
                    }
                } else {
                    LOG_ERROR("RT Engine failed to remove track");
                }
        });
    } else {
        LOG_ERROR("Failed to remove track from render graph");
    }
    
}
END_HANDLE

END_BLOCK