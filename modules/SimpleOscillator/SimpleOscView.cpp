// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modules/SimpleOscillator/SimpleOscView.h"
#include "modules/SimpleOscillator/SimpleOsc.h"
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
    
}