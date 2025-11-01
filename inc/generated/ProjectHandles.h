/* This file is generated automatically, do not edit manually */
#pragma once
#include "core/ModuleManager.h"
#include "snapshots/AudioUnitView.h"
#include "core/primitives/AudioUnit.h"
#include "ui/uiControls.h"
#include "core/ControlEngine.h"
#include "core/primitives/RenderPlan.h"
#include "logger.h"
#include "snapshots/ProjectView.h"
#include "core/Project.h"
#include "Status.h"
#include "core/primitives/ControlContext.h"

namespace slr {

inline void handleEvent(const ControlContext &ctx, const Events::AddNewRoute &e) {
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

inline void handleEvent(const ControlContext &ctx, const Events::DeleteModule &e) {
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

inline void handleEvent(const ControlContext &ctx, const Events::CreateModule &e) {
    const Module * mod = ModuleManagerFactory::findModule(e.name);
    if(!mod) {
        LOG_ERROR("Failed to find module %s", e.name.c_str());
        return;
    }

    AudioUnit * au = nullptr;
    AudioUnitView * view = nullptr;

    try {
        std::unique_ptr<AudioUnit> unit = mod->createRT();
        au = unit.get();
        ctx.project->addUnit(std::move(unit));
        
        view = mod->createView(au);
        ctx.projectView->addUnitView(view);
    } catch(...) {
        LOG_ERROR("Failed to create module %s", e.name.c_str());
        return;
    }

    if(!au && !view) {
        LOG_ERROR("2 Failed to create module %s", e.name.c_str());
        return;
    }

    UIControls::addModuleUI(mod, view);
}

} //namespace slr
