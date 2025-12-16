// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modules/Mixer/MixerView.h"
#include "modules/Mixer/Mixer.h"

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

}