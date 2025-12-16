// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK

EV
INCLUDE <string>
struct CreateModule {
    std::string name;
};

EV_HANDLE
INCLUDE "core/ModuleManager.h"
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
END_HANDLE

END_BLOCK