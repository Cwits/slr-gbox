// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"
#include "core/primitives/ParameterArray.h"
#include "core/primitives/Parameter.h"
#include "core/primitives/AudioBuffer.h"
#include "core/primitives/FileContainer.h"
#include "core/FlatEvents.h"
#include "Status.h"

namespace slr {

class AudioContext;
class ParameterFloat;
class ParameterBool;
class ParameterInt;
class ParameterBase;
class Dependencies;

enum class AudioUnitType {
    Error,
    Mixer,
    Track,
    Effect,
    Metronome,
    // EffectChain,
};

class AudioUnit {
    public:
    explicit AudioUnit(AudioUnitType type, bool needsAudioOutput = true);
    virtual ~AudioUnit();

    RT_FUNC virtual frame_t process(const AudioContext &ctx, const Dependencies * const inputs, const uint32_t inputsCount) = 0;

    RT_FUNC virtual void prepareToPlay() = 0;
    RT_FUNC virtual void prepareToRecord() = 0;
    RT_FUNC virtual void stopPlaying() = 0;
    RT_FUNC virtual void stopRecording() = 0;

    RT_FUNC virtual frame_t latency() { return 0; }

    RT_FUNC static Status setParameter(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp);
    
    struct UnitOutput {
        AudioBuffer * ptr;
        std::string &name;
        uint32_t _outputId;
    };

    // virtual const std::vector<UnitOutput> & outputs() const;
    // virtual const AudioBuffer * output(uint32_t outputId) const;
    virtual const AudioBuffer * outputs() const { return _outputs; }

    //void injectMidi(...);
    //void injectControl(...);

    const std::vector<ContainerItem*> * items() const { return _fileContainer._items; }
    RT_FUNC static Status appendItem(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp);
    RT_FUNC static Status swapContainer(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp);
    RT_FUNC static Status modifyContainerItem(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp);

    const ID id() const { return _uniqueId; }
    
    // const AudioUnitType type() const { return _type; }
    // const bool solo() const { return _solo; }

    void setMute(bool mute) { *_mute = mute; }
    const bool mute() const { return *_mute; }
    const ID muteId() const { return _mute->id(); }
    
    void setVolume(float volume) { *_volume = volume; }
    const float volume() const { return *_volume; }
    const ID volumeId() const { return _volume->id(); }

    void setPan(float pan) { *_pan = pan; }
    const float pan() const { return *_pan; }
    const ID panId() const { return _pan->id(); }

    bool hasParameterWithId(ID parameterId);

    protected:
    AudioUnitType _type;
    void addParameter(ParameterBase * base);

    bool _solo;
    bool _sendToMixer;
    
    ParameterFloat * _volume;
    ParameterFloat * _pan;
    ParameterBool * _mute;
    ParameterArray _flatParameterList;
    
    const ID _uniqueId = -1;

    const bool _haveAudioOutputs;
    AudioBuffer * _outputs;

    RT_FUNC void playbackFiles(const AudioContext &ctx, AudioBuffer *buf/*, MidiBuffer *mid */);
    FileContainer _fileContainer;

    //spsc queue midi
    //spsc queue control

    //midi learn map
    //automation clips

    friend class AudioUnitView;
};

    
}