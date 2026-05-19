// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/Mixer/MixerView.h"
#include "units/Mixer/Mixer.h"
#include "units/Mixer/MixerUnit.h"

namespace slr {

MixerView::MixerView(Mixer * const mix) :
    AudioUnitView(mix),
    _mixer(mix)
{
    _name = "Mixer " + std::to_string(_uniqueId);
}

MixerView::~MixerView() {

}

void MixerView::update() {
    AudioUnitView::update();
}


std::string MixerView::unitType() { 
    std::string ret = "internal:";
    ret += *MixerDescriptor._name;
    return ret;
}

}