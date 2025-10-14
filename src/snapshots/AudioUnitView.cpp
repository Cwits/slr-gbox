// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "snapshots/AudioUnitView.h"
#include "core/primitives/AudioUnit.h"

namespace slr {

AudioUnitView::AudioUnitView(AudioUnit * au) : 
    _au(au),
    _uniqueId(au->id()) 
    // _solo(au->solo()),

{
    addParameter(_volume = new ParameterFloatView(au->_volume));
    addParameter(_pan = new ParameterFloatView(au->_pan));
    addParameter(_mute = new ParameterBoolView(au->_mute));

    
    _name = "Untitled ";
    _name.append(std::to_string(au->id()));
    _uniqueColor.r = rand()%255;
    _uniqueColor.g = rand()%255;
    _uniqueColor.b = rand()%255;
    _uniqueColor.a = 255;

    _solo = false;
}

AudioUnitView::~AudioUnitView() {
    
}

void AudioUnitView::addParameter(ParameterBaseView * base) {
    _flatParameterList.add(base);
}

void AudioUnitView::update() {
    // _solo = *_au->solo();
    *_volume = _au->volume();
    *_pan = _au->pan();
    *_mute = _au->mute();
}
    
}