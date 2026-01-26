/* This file is generated automatically, do not edit manually */
#pragma once
#include "core/primitives/RenderPlan.h"
#include "Status.h"
#include "core/primitives/FileContainer.h"
#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"
#include "logger.h"
#include "ui/uiControls.h"
#include "core/ModuleManager.h"
#include "core/primitives/AudioUnit.h"
#include "core/Project.h"
#include "core/ControlEngine.h"
#include "core/primitives/ControlContext.h"

namespace slr {

inline void handleEvent(const ControlContext &ctx, const Events::AddNewRoute &e) {
    if(!ctx.project->evaluateRoute(e.route)) {
        UIControls::floatingWarning("Invalid route");
        LOG_ERROR("Invalid route");
        return;
    }
    
    ctx.project->addRoute(e.route);

    RenderPlan * newPlan = buildPlan(ctx.project);
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

inline void handleEvent(const ControlContext &ctx, const Events::AddNewMidiRoute &e) {
    // if(!ctx.project->evaluateRoute(e.route)) {
    //     UIControls::floatingWarning("Invalid route");
    //     LOG_ERROR("Invalid route");
    //     return;
    // }
    
    ctx.project->addRoute(e.route);

    RenderPlan * newPlan = buildPlan(ctx.project);
    if(newPlan == nullptr) {
        UIControls::floatingWarning("Failed to build new plan");
        LOG_ERROR("Failed to build new plan");
        LOG_WARN("Must remove last added route");
        return;
    }

    ctx.project->replaceEditablePlan(newPlan);
    ctx.projectView->updateRoutes(ctx.project->midiRoutes());

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

        LOG_INFO("Midi Route added successfully");
        return;
        
        error:
        LOG_ERROR("Adding midi route gone wrong");
        //cleanup
        //notify user
        return;
    });
}

inline void handleEvent(const ControlContext &ctx, const Events::DeleteModule &e) {
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

inline void handleEvent(const ControlContext &ctx, const Events::CreateModule &e) {
    const Module * mod = ModuleManagerFactory::findModule(e.name);
    if(!mod) {
        LOG_ERROR("Failed to find module %s", e.name.c_str());
        return;
    }


    try {
        AudioUnit * au = nullptr;
        AudioUnitView * view = nullptr;
        std::unique_ptr<AudioUnit> unit = mod->createRT();
        au = unit.get();

        if(!unit->create(ctx.bufferManager)) {
            LOG_ERROR("Failed to create unit for some reasons");
            return;
        }

        ctx.project->addUnit(std::move(unit));

        view = mod->createView(au);
        ctx.projectView->addUnitView(view);

        if(!au && !view) {
            LOG_ERROR("2 Failed to create module %s", e.name.c_str());
            return;
        }

        ClipContainerMap &map = ctx.project->clipContainerMap();
        auto [it, inserted] = map.try_emplace(au->id());
        ClipStorage & storage = it->second;

        au->setClips(storage.getOtherVector(nullptr));
        
        if(!inserted) LOG_ERROR("Clip Container Map with %u id already exists", au->id());

        UIControls::addModuleUI(mod, view);
    } catch(...) {
        LOG_ERROR("Failed to create module %s", e.name.c_str());
        return;
    }

}

} //namespace slr
