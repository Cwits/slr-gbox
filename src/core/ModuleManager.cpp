// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/ModuleManager.h"
#include "core/primitives/AudioUnit.h"
#include "ui/display/primitives/UnitUIBase.h"

#include "modules/Track/TrackModule.h"
#include "modules/Mixer/MixerModule.h"

#include <vector>
#include <string>

namespace slr {

void ModuleManagerFactory::init() {
    ModuleManagerFactory & inst = ModuleManagerFactory::inst();
    //init default
    inst.registerModule(&TrackModule);
    inst.registerModule(&MixerModule);


    //inst.discoverModules();
}

void ModuleManagerFactory::discoverModules() {

}

const Module * ModuleManagerFactory::findModule(std::string name) {
    std::vector<const Module *> &list = inst()._moduleList;
    for(std::size_t m=0; m<list.size(); ++m) {
        const Module * mod = list.at(m);
        if(mod->_name->compare(name) == 0) {
            return mod;
        }
    }

    return nullptr;
}

} //slr