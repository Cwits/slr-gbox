// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/SimpleOscillator/SimpleOscView.h"
#include "units/SimpleOscillator/SimpleOsc.h"
#include "units/SimpleOscillator/SimpleOscUnit.h"
#include "core/utility/helper.h"


namespace slr {


SimpleOscView::SimpleOscView(SimpleOsc * osc) : 
    AudioUnitView(osc), 
    _osc(osc) 
{
    _name = "OSC " + std::to_string(_uniqueId);
}

SimpleOscView::~SimpleOscView() {

}

void SimpleOscView::update() {
    AudioUnitView::update();

    //update File container
}

std::string SimpleOscView::unitType() { 
    std::string ret = "internal:";
    ret += *SimpleOscDescriptor._name;
    return ret;
}
    
nlohmann::ordered_json SimpleOscView::saveUnit() {
    nlohmann::ordered_json ret;

    ret = AudioUnitView::saveUnit();

    ret["Additional Parameters"]["Par 1"] = 0.0001;
    ret["Additional Parameters"]["Par 2"] = true;
    ret["Additional Parameters"]["Par 3"] = "abazubaka";
    ret["Additional Parameters"]["Par 4"] = 0.998415;
    
    return ret;
}

}