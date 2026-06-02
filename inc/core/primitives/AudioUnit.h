// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"
#include "core/primitives/ParameterArray.h"
#include "core/primitives/Parameter.h"
#include "core/primitives/FileContainer.h"
#include "core/primitives/AudioBuffer.h"
#include "core/primitives/MidiBuffer.h"
#include "common/Status.h"

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
    AudioUnit(const ClipContainer *initialContainer, ID id);
    virtual ~AudioUnit();

    virtual bool create(BufferManager *man);
    virtual bool destroy(BufferManager *man);

    // virtual void reinit() {}

    RT_FUNC virtual frame_t process(const AudioContext &ctx, const Dependencies &inputs) = 0;

    RT_FUNC virtual void prepareToPlay() = 0;
    RT_FUNC virtual void prepareToRecord() = 0;
    RT_FUNC virtual void stopPlaying() = 0;
    RT_FUNC virtual void stopRecording() = 0;

    RT_FUNC virtual frame_t latency() { return 0; }

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
    inline void setMidiThru(bool state) { _midiThru = state; }
    
    const bool isOmniHwInput() const { return _omniHwInput; }
    inline void setOmniHw(bool state) { _omniHwInput = state; }
    
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

    inline void setParameter(ID parameterId, float value) {
        _flatParameterList[parameterId]->setValue(value);
    }
    
    bool hasParameterWithId(ID parameterId);

    const ClipContainer * clips() const { return _clipContainer; }
    inline void setClipContainer(const ClipContainer *cont) { _clipContainer = cont; }
    
    static ID nextAudioUnitId();

    protected:
    const ID _uniqueId;

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

    RT_FUNC void applyMidiEvents(MidiBuffer *buf);

    RT_FUNC void playbackFiles(const AudioContext &ctx, AudioBuffer *buf, MidiBuffer *mid);
    const ClipContainer *_clipContainer;
    
    friend class AudioUnitView;
    friend class LoadProject;
};
    
}
