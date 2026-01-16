// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"
#include "core/primitives/ParameterArray.h"
#include "core/primitives/Parameter.h"
#include "core/primitives/FileContainer.h"
#include "core/primitives/AudioBuffer.h"
#include "core/primitives/MidiBuffer.h"
#include "core/FlatEvents.h"
#include "Status.h"

#include <vector>

namespace slr {

class AudioContext;
class ParameterFloat;
class ParameterBool;
class ParameterInt;
class ParameterBase;
class Dependencies;
class BufferManager;

class AudioUnit {
    public:
    explicit AudioUnit();
    //explicit AudioUnit(ClipContainer & container, bool needsAudioOutput = true);
    virtual ~AudioUnit();

    virtual bool create(BufferManager *man);
    virtual bool destroy(BufferManager *man);

    RT_FUNC virtual frame_t process(const AudioContext &ctx, const Dependencies &inputs) = 0;

    RT_FUNC virtual void prepareToPlay() = 0;
    RT_FUNC virtual void prepareToRecord() = 0;
    RT_FUNC virtual void stopPlaying() = 0;
    RT_FUNC virtual void stopRecording() = 0;

    RT_FUNC virtual frame_t latency() { return 0; }

    RT_FUNC static Status setParameter(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp);

    RT_FUNC static Status toggleMidiThru(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp);
    RT_FUNC static Status toggleOmniHwInput(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp);
    
    struct UnitOutput {
        AudioBuffer * ptr;
        std::string &name;
        uint32_t _outputId;
    };

    // virtual const std::vector<UnitOutput> & outputs() const;
    // virtual const AudioBuffer * output(uint32_t outputId) const;
    virtual const AudioBuffer * outputs() const { return _outputs; }

    // void injectMidi(MidiEvent & ev) { _midiQueue.push_back(ev); }
    void clearMidiBuffer();
    MidiBuffer * midiOutputs() { return _midiOutput; }
    const bool isMidiThru() const { return _midiThru; }
    const bool isOmniHwInput() const { return _omniHwInput; }
    
    // [[deprecated("FOR TEST PURPOSE ONLY")]]
    // void setMidiThru(bool newState) { _midiThru = newState; } //for test purposes, use events instead
    //void injectControl(...);

    const ID id() const { return _uniqueId; }
    
    // const AudioUnitType type() const { return _type; }
    // const bool solo() const { return _solo; }

    void setMute(bool mute) { _mute = mute; }
    const bool mute() const { return _mute; }
    const ID muteId() const { return _mute.id(); }
    
    void setVolume(float volume) { _volume = volume; }
    const float volume() const { return _volume; }
    const ID volumeId() const { return _volume.id(); }

    void setPan(float pan) { _pan = pan; }
    const float pan() const { return _pan; }
    const ID panId() const { return _pan.id(); }

    bool hasParameterWithId(ID parameterId);

    bool checkFileContainerNeedResize();
    void resizeFileContainer();

    const std::vector<ClipItem*> * clips() const { return _clipContainer.clips(); }
    RT_FUNC static Status appendItem(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp);
    RT_FUNC static Status swapContainer(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp);
    RT_FUNC static Status modifyClipItem(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp);
    
    void setClips(std::vector<ClipItem*> * ptr) { _clipContainer._clips = ptr; }

    protected:
    const ID _uniqueId = -1;

    bool _solo;
    
    void addParameter(ParameterBase * base);
    ParameterArray _flatParameterList;
    ParameterFloat _volume;
    ParameterFloat _pan;

    ParameterBool _mute;
    bool _buffersClear;
    bool isMuted(const AudioContext &ctx);

    AudioBuffer * _outputs;

    bool _midiThru;
    bool _omniHwInput; //unit will gather all available midi inputs event if there is no route for that
    // std::vector<MidiEvent> _midiQueue;
    MidiBuffer * _midiInput;
    MidiBuffer * _midiOutput;
    //spsc queue midi
    //spsc queue control

    //midi learn map
    //automation clips

    RT_FUNC void playbackFiles(const AudioContext &ctx, AudioBuffer *buf/*, MidiBuffer *mid */);
    ClipContainer _clipContainer;
    
    friend class AudioUnitView;
};

    
}
