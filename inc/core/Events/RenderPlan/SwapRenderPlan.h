// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK SwapRenderPlan

FLAT_REQ
struct SwapRenderPlan {
    Project * project; //-> class Project;
};

RT_HANDLE
INCLUDE "core/Project.h"
&Project::swapPlan
END_HANDLE

RESP_HANDLE 
INCLUDE "core/primitives/AudioUnit.h"
INCLUDE "core/Project.h"
INCLUDE "logger.h"
void handleGraphSwapped(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
    if(resp.status == Status::Ok) {        
        // const std::vector<AudioUnit*> & order = ctx.project->renderGraph().order();
        // std::vector<AudioUnit*> & editable = ctx.project->renderGraph().editableGraph();

        // editable = order;
        LOG_INFO("Render plan swapped successfully");
    } else {
        LOG_ERROR("Failed to swap render graph");    
    }
}
END_HANDLE

END_BLOCK