/* This file is generated automatically, do not edit manually */
#pragma once
#include "core/Project.h"
#include "core/primitives/AudioUnit.h"
#include "logger.h"
#include "core/primitives/ControlContext.h"

namespace slr {

inline void handleGraphSwapped(const ControlContext &ctx, const FlatEvents::FlatResponse & resp) {
    if(resp.status == Status::Ok) {        
        // const std::vector<AudioUnit*> & order = ctx.project->renderGraph().order();
        // std::vector<AudioUnit*> & editable = ctx.project->renderGraph().editableGraph();

        // editable = order;
        LOG_INFO("Render plan swapped successfully");
    } else {
        LOG_ERROR("Failed to swap render graph");    
    }
}

} //namespace slr
