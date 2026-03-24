// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK

EV
INCLUDE <string>
struct CreateModule {
    std::string name;
};

EV_HANDLE
INCLUDE "core/primitives/ControlContext.h"
INCLUDE "core/ModuleManager.h"
INCLUDE "core/primitives/FileContainer.h"
INCLUDE "core/Project.h"
INCLUDE "core/primitives/AudioUnit.h"
INCLUDE "snapshots/AudioUnitView.h"
INCLUDE "ui/uiControls.h"
INCLUDE "logger.h"
void handleEvent(const ControlContext &ctx, const Events::CreateModule &e) {
    const Module * mod = ModuleManagerFactory::findModule(e.name);
    if(!mod) {
        LOG_ERROR("Failed to find module %s", e.name.c_str());
        return;
    }

    if(ctx.prohibitAllocation()) {
        LOG_ERROR("Low on memory");
        return;
    }
    
    AudioUnit * au = ctx.project->createUnit(ctx, mod);
    if(!au) {
        //error
        LOG_ERROR("Failed to create %s RT", e.name.c_str());
        return;
    }

    AudioUnitView *view = ctx.projectView->createUnitView(ctx, mod, au);
    if(!view) {
        LOG_ERROR("2 Failed to create module %s", e.name.c_str());
        //ctx.project->discardUnit(au);
        return;
    }

    UIControls::addModuleUI(mod, view);
}
END_HANDLE

END_BLOCK