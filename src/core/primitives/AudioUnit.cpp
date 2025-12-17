// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/AudioUnit.h"

#include "core/primitives/Parameter.h"
#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioContext.h"

#include "core/AudioBufferManager.h"
#include "logger.h"

namespace slr {

/* 
//use 1024 this as starting point for counting ID's for now, 
because there is some problem with getSourcesForId() and getTargetsForId() ->
result yelds not existing routes(at least for midi...)
*/    
constexpr int FirstUnitId = 0;
static ID uniqueIdCounter = FirstUnitId; 

AudioUnit::AudioUnit(AudioUnitType type, bool needsAudioOutputs) : _type(type), _uniqueId(uniqueIdCounter++), _haveAudioOutputs(needsAudioOutputs) {
    addParameter(_volume = new ParameterFloat("Volume", 1.0f, 0.0f, 1.0f));
    addParameter(_pan = new ParameterFloat("Pan", 0.5f, 0.f, 1.f));
    addParameter(_mute = new ParameterBool("Mute", 0.0f, 0.f, 1.f));

    if(needsAudioOutputs) {
        _outputs = AudioBufferManager::acquireRegular();
    }

    _midiQueue.reserve(MIDI_SPSCQUEUE_SIZE);
}

AudioUnit::~AudioUnit() {
    if(_haveAudioOutputs) {
        AudioBufferManager::releaseRegular(_outputs);
    }
}

void AudioUnit::addParameter(ParameterBase * base) {
    _flatParameterList.add(base);
}

bool AudioUnit::hasParameterWithId(ID parameterId) {
    const std::size_t count = _flatParameterList.count();
    for(std::size_t i=0; i<count; ++i) {
        if(_flatParameterList[i]->id() == parameterId)
            return true;
    }

    return false;
}

Status AudioUnit::setParameter(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp) { 
    AudioUnit * u = ev.setParameter.unit;
    u->_flatParameterList[ev.setParameter.parameterId]->setValue(ev.setParameter.value);
    
    resp.type = FlatEvents::FlatResponse::Type::SetParameter;
    resp.status = Status::Ok;
    resp.commandId = ev.commandId;
    resp.setParameter.unit = u;
    resp.setParameter.parameterId = ev.setParameter.parameterId;
    resp.setParameter.value = ev.setParameter.value;
    return Status::Ok;
}

Status AudioUnit::toggleMidiThru(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp) {
    AudioUnit *u = ev.toggleMidiThru.unit;
    u->_midiThru = ev.toggleMidiThru.newState;

    resp.type = FlatEvents::FlatResponse::Type::ToggleMidiThru;
    resp.status = Status::Ok;
    resp.commandId = ev.commandId;
    resp.toggleMidiThru.unit = u;
    resp.toggleMidiThru.newState = ev.toggleMidiThru.newState;
    return Status::Ok;
}

Status AudioUnit::toggleOmniHwInput(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp) {
    AudioUnit *u = ev.toggleOmniHwInput.unit;
    u->_omniHwInput = ev.toggleOmniHwInput.newState;

    resp.type = FlatEvents::FlatResponse::Type::ToggleOmniHwInput;
    resp.status = Status::Ok;
    resp.commandId = ev.commandId;
    resp.toggleOmniHwInput.unit = u;
    resp.toggleOmniHwInput.newState = ev.toggleOmniHwInput.newState;
    return Status::Ok;
}


}