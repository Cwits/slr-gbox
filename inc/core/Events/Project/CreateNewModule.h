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
END_HANDLE

END_BLOCK