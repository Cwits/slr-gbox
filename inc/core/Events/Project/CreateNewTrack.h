// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK NewTrack

EV
struct NewTrack {
};

EV_HANDLE
INCLUDE "core/primitives/AudioUnit.h"
INCLUDE "core/ControlEngine.h"
INCLUDE "core/Project.h"
INCLUDE "snapshots/ProjectView.h"
INCLUDE "ui/uiControls.h"
INCLUDE "core/primitives/RenderPlan.h"
INCLUDE "core/Mixer.h"
INCLUDE "core/ModuleManager.h"
INCLUDE "modules/Track/Track.h"
INCLUDE "modules/Track/TrackView.h"
INCLUDE "modules/Track/TrackUI.h"
void handleEvent(const ControlContext &ctx, const Events::NewTrack &e) {
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
END_HANDLE

END_BLOCK