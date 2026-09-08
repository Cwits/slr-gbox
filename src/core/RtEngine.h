// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "common/defines.h"
#include "core/primitives/AudioBuffer.h"
#include "core/primitives/SPSCQueue.h"
#include "core/primitives/MidiEvent.h"
#include "core/primitives/MidiRtStructs.h"
#include "common/Status.h"

#include <memory>
#include <atomic>
#include <array>
#include <vector>
#include <unordered_map>
#include <functional>
/* Sample. Loop. Repeat. */
namespace slr {

class AudioDriver;
// class AudioBuffer;
// class Project;
struct RtTask;
struct RenderPlan;

class RtEngine {
    public:
    enum class RtState { ERROR, STOP, RUN };
    RtEngine();
    ~RtEngine();

    bool init();
    bool shutdown();
    bool start(std::function<void(frame_t)> anchorLambda);
    bool stop();

    const RtState getState() const { return _state; } 
    
    void SwapRenderPlan(const RenderPlan * plan);

    void addTask(RtTask * task);

    const int blockSize() const;
    const int channels() const;

    AudioDriver * driver() const { return _driver.get(); }

    const std::vector<RtMidiBuffer> * midiLocalBuffers() const { return _midiInLocal; }
    const std::vector<RtMidiQueue> * midiInMap() const { return _midiInputMap; }
    const std::vector<RtMidiOutput> * midiOutMap() const { return _midiOutputMap; }

    void setMidiLocal(std::vector<RtMidiBuffer> * buf);
    void setMidiIn(std::vector<RtMidiQueue> *buf);
    void setMidiOut(std::vector<RtMidiOutput> *buf);
    // static Common::Status updateMidiMaps(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp);

    static void addRtResponse(RtTask * task);
    SPSCQueue<RtTask*, 256> & getResponses() { return _rtResponses; }
    // void processResponses();
    
    private:
    frame_t processNextBlock(AudioBuffer * inputs, AudioBuffer * outputs, frame_t frames, frame_t framesPassed);

    std::unique_ptr<AudioDriver> _driver;
    // Project * _prj;
    const RenderPlan * _plan;

    SPSCQueue<RtTask*, 256> _rtTasks;
    SPSCQueue<RtTask*, 256> _rtResponses;

    std::vector<RtMidiBuffer> * _midiInLocal;
    std::vector<RtMidiQueue> *_midiInputMap;
    std::vector<RtMidiOutput> *_midiOutputMap;
    
    std::atomic<RtState> _state;

    bool _isFirstCallback;
};

}