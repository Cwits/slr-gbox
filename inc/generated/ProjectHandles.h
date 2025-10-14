/* This file is generated automatically, do not edit manually */
#pragma once
#include "core/ModuleManager.h"
#include "core/primitives/RenderPlan.h"
#include "snapshots/ProjectView.h"
#include "ui/uiControls.h"
#include "snapshots/AudioUnitView.h"
#include "core/Mixer.h"
#include "Status.h"
#include "modules/Track/TrackView.h"
#include "core/primitives/AudioUnit.h"
#include "modules/Track/Track.h"
#include "logger.h"
#include "modules/Track/TrackUI.h"
#include "core/Project.h"
#include "core/ControlEngine.h"
#include "core/primitives/ControlContext.h"

namespace slr {

inline void handleEvent(const ControlContext &ctx, const Events::NewTrack &e) {
    (void)e;
    
    bool noErrors = true;
    const Module * mod = ModuleManagerFactory::findModule("Track");
    std::unique_ptr<AudioUnit> track = mod->createRT();
    AudioUnit * unit = static_cast<AudioUnit*>(track.get());

    noErrors &= ctx.project->addUnit(std::move(track));
    
    AudioRoute routeToMix;
    routeToMix._sourceType = AudioRoute::Type::INT;
    routeToMix._sourceId = unit->id();
    routeToMix._targetType = AudioRoute::Type::INT;
    routeToMix._targetId = ctx.project->mixer()->id();
    for(int i=0; i<32; ++i) {
        routeToMix._channelMap[i] = -1;
    }
    routeToMix._channelMap[0] = 0;
    routeToMix._channelMap[1] = 1;
    if(!ctx.project->evaluateRoute(routeToMix)) {
        return;
    }
    ctx.project->addRoute(routeToMix);

    ctx.projectView->updateRoutes(ctx.project->routes());

    RenderPlan * newPlan = buildPlan(ctx.project, ctx.project->routes());
    ctx.project->replaceEditablePlan(newPlan);

    if(noErrors) {
        FlatEvents::FlatControl ctl;
        ctl.type = FlatEvents::FlatControl::Type::SwapRenderPlan;
        ctl.swapRenderPlan.project = ctx.project;
        ctl.commandId = ControlEngine::generateCommandId();
     
        ControlEngine::awaitRtResult(ctl, [trackid = unit->id(), mod](const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
            if(resp.status == Status::Ok) {
                AudioUnit * u = ctx.project->getUnitById(trackid);
                Track * tr = dynamic_cast<Track*>(u);
                if(tr == nullptr) goto error;

                TrackView * view = new TrackView(tr);
                ctx.projectView->addUnitView(view);
                
                UIControls::addModuleUI(mod, view);
            } else {
                goto error;
                //clear memory in project
                //notify user
            }

            return;

            error:
            LOG_ERROR("Track adding gone wrong");
            //cleanup
            //notify user
            return;
        });
    } else {
        //cleanup
        //notifu user
    }

    
}

inline void handleEvent(const ControlContext &ctx, const Events::DeleteTrack &e) {
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

} //namespace slr
