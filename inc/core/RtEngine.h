// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "defines.h"
#include "core/primitives/AudioBuffer.h"
#include "core/primitives/AudioContext.h"
#include "core/primitives/SPSCQueue.h"
#include "core/primitives/MidiEvent.h"
#include "core/FlatEvents.h"
#include "Status.h"

#include <memory>
#include <atomic>
#include <array>
#include <vector>
#include <unordered_map>
/* Sample. Loop. Repeat. */
namespace slr {

class AudioDriver;
// class AudioBuffer;
class Project;

class RtEngine {
    public:
    enum class RtState { ERROR, STOP, RUN };
    RtEngine();
    ~RtEngine();

    bool init();
    bool shutdown();
    bool start();
    bool stop();

    const RtState getState() const { return _state; } 
    void setProject(Project * prj);
    bool FlatControlEvent(const FlatEvents::FlatControl &ev) { return _inputControl.push(ev); }
    bool FlatResponseEvent(const FlatEvents::FlatResponse &resp) { return _outputControl.push(resp); }
    
    SPSCQueue<FlatEvents::FlatResponse, 256> & getOutputQueue() { return _outputControl; }

    const int blockSize() const;
    const int channels() const;

    AudioDriver * driver() const { return _driver.get(); }

    static void addRtControl(const FlatEvents::FlatControl &ctl);
    static void addRtResponse(const FlatEvents::FlatResponse & resp);

    const std::vector<RtMidiBuffer> * midiLocalBuffers() const { return _midiInLocal; }
    const std::vector<RtMidiQueue> * midiInMap() const { return _midiInputMap; }
    const std::vector<RtMidiOutput> * midiOutMap() const { return _midiOutputMap; }

    static Status updateMidiMaps(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp);

    private:
    frame_t processNextBlock(AudioBuffer * inputs, AudioBuffer * outputs, frame_t frames, frame_t framesPassed);
    void handleControlEvents(std::array<FlatEvents::FlatControl, 256> & list, frame_t & framesPassed);

    std::unique_ptr<AudioDriver> _driver;
    Project * _prj;
    SPSCQueue<FlatEvents::FlatControl, 256> _inputControl;
    SPSCQueue<FlatEvents::FlatResponse, 256> _outputControl;
    std::array<FlatEvents::FlatControl, 256> _controlSnapshot;
    int _snapshotCount;

    std::vector<RtMidiBuffer> * _midiInLocal;
    std::vector<RtMidiQueue> *_midiInputMap;
    std::vector<RtMidiOutput> *_midiOutputMap;
    
    std::atomic<RtState> _state;

};

}