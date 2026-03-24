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
    addParameter(_volume = new ParameterFloatView(&au->_volume));
    addParameter(_pan = new ParameterFloatView(&au->_pan));
    addParameter(_mute = new ParameterBoolView(&au->_mute));

    
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

void AudioUnitView::setParameter(ID parameterId, float value) { 
    _flatParameterList.operator[](parameterId)->setValue(value); 
    incrementVersion();
}

void AudioUnitView::appendClipItem(ClipItemView * item) {
    // _clipContainer._items.push_back(item);
    _clipContainer.addClipItem(item);
    incrementVersion();
} 

// void AudioUnitView::update() {
//     // _solo = *_au->solo();
//     *_volume = _au->volume();
//     *_pan = _au->pan();
//     *_mute = _au->mute();
//     _midiThru = _au->isMidiThru();
//     _omniHwInput = _au->isOmniHwInput();
//     incrementVersion();
// }

// void AudioUnitView::setMute(bool mute) {
//     *_mute = mute; incrementVersion();
// }

// void AudioUnitView::setVolume(float volume) {
//     *_volume = volume; incrementVersion();
// }

// void AudioUnitView::setPan(float pan) {
//     *_pan = pan; incrementVersion();
// }

void AudioUnitView::incrementVersion() {
    _version.fetch_add(1, std::memory_order_acq_rel);
}

uint64_t AudioUnitView::version() const { 
    return _version.load(std::memory_order_acquire);
}


    
}