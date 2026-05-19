// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/UnitManager.h"
#include "core/primitives/AudioUnit.h"
#include "ui/display/primitives/UnitUIBase.h"

#include "core/ActionsMap.h"

#include "units/Track/TrackUnit.h"
#include "units/Mixer/MixerUnit.h"
#include "units/SimpleOscillator/SimpleOscUnit.h"
#include "units/Sampler/SamplerUnit.h"

#include <vector>
#include <string>

namespace slr {

void UnitManagerFactory::init() {
    UnitManagerFactory & inst = UnitManagerFactory::inst();
    //init default
    
    std::map<std::type_index, CreatorFn> &map = getActionMap();
    
    registerDefaultActions(map);

    //this should happen during application startup - discover all possible units and test them, only than register as available
    registerTrackActions(map);
    inst.registerUnit(&TrackDescriptor);
    
    inst.registerUnit(&MixerDescriptor);
    
    inst.registerUnit(&SimpleOscDescriptor);

    registerSamplerActions(map);
    inst.registerUnit(&SamplerDescriptor);


    //inst.discoverModules();
}

void UnitManagerFactory::discoverUnits() {

}

const UnitDescriptor * UnitManagerFactory::findUnit(std::string name) {
    std::vector<const UnitDescriptor *> &list = inst()._unitDescriptorList;
    for(std::size_t m=0; m<list.size(); ++m) {
        const UnitDescriptor * desc = list.at(m);
        if(desc->_name->compare(name) == 0) {
            return desc;
        }
    }

    return nullptr;
}

} //slr