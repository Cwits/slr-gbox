// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK AddNewRoute

EV  
INCLUDE "core/primitives/AudioRoute.h"
struct AddNewRoute {
    AudioRoute route;
};

EV_HANDLE
INCLUDE "core/Project.h"
INCLUDE "snapshots/ProjectView.h"
INCLUDE "ui/uiControls.h"
INCLUDE "core/primitives/RenderPlan.h"
INCLUDE "core/ControlEngine.h"
void handleEvent(const ControlContext &ctx, const Events::AddNewRoute &e) {
    if(!ctx.project->evaluateRoute(e.route)) {
        UIControls::floatingWarning("Invalid route");
        LOG_ERROR("Invalid route");
        return;
    }
    
    ctx.project->addRoute(e.route);

    RenderPlan * newPlan = buildPlan(ctx.project, ctx.project->routes());
    if(newPlan == nullptr) {
        UIControls::floatingWarning("Failed to build new plan");
        LOG_ERROR("Failed to build new plan");
        LOG_WARN("Must remove last added route");
        return;
    }

    ctx.project->replaceEditablePlan(newPlan);
    ctx.projectView->updateRoutes(ctx.project->routes());

    FlatEvents::FlatControl ctl;
    ctl.type = FlatEvents::FlatControl::Type::SwapRenderPlan;
    ctl.swapRenderPlan.project = ctx.project;
    ctl.commandId = ControlEngine::generateCommandId();
     
    ControlEngine::awaitRtResult(ctl, [](const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
        if(resp.status == Status::Ok) {
            UIControls::updateRouteManager();
        } else {
            goto error;
            //clear memory in project
            //notify user
        }

        LOG_INFO("Route added successfully");
        return;
        
        error:
        LOG_ERROR("Adding route gone wrong");
        //cleanup
        //notify user
        return;
    });
}
END_HANDLE

END_BLOCK