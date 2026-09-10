// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "common/defines.h"
#include "core/primitives/StepEvent.h"
 
#include <vector>
#include <memory>
#include <array>

namespace slr {

struct AudioUnit;
struct AudioContext;
struct Timeline;

const int TARGET_COUNT = 8;
const int EVENTS_COUNT = 256;
const int LAYERS_COUNT = 16;
const int STEPS_PER_PAGE = 16;

struct Sequence {
    Sequence(ID uniqueId);

    void prepareToPlay() const;
    void stopAfterPlay() const;

    frame_t process(const AudioContext &ctx) const;
    void onFlySwap(const Sequence *original) const {} //copy the refpoint, framesbeforenoteoff and lastplayedstep

    const ID _uniqueId;

    std::array<const AudioUnit*, TARGET_COUNT> _targets;
    uint32_t _stepCount;
    
    struct Layer {
        const AudioUnit *_target; //if nullptr than send to _targets, else use this one
        std::array<StepEvent, EVENTS_COUNT> _events;
        uint8_t _note;
        uint8_t _velocity;

        bool _mute;
        bool _active;

        mutable bool _eventTriggered;
        mutable uint32_t _lastTriggeredEvent; 
        
        void initAllTo(MidiEvent ev);
        void clear();
        const MidiEvent findLastActive(unsigned int current) const;
    };
    
    std::array<Layer, LAYERS_COUNT> _layers;
    // std::array<frame_t, EVENTS_COUNT> _eventPositions;

    void initDefault(const Timeline &tl);
    // void recalculateEventPositions(const Timeline &tl, StepDuration newDuration); 
    void initLayer(int layer, int note, int velocity);
    void clearAllLayers();
    void clearLayer(int id);
    void clear();

    StepDuration stepDuration() const { return _stepDuration; }
    void setDuration(StepDuration dur) { _stepDuration = dur; }

    mutable frame_t _framesTillNextStep;
    mutable frame_t _framesTillNoteOff;
    // mutable bool _noteTriggered;

    private:
    StepDuration _stepDuration;
};

struct StepSequencerEngine {
    StepSequencerEngine();
    ~StepSequencerEngine();

    Sequence * createNewSequence();
    // Sequence * cloneSequence(const Sequence *seq);

    std::unique_ptr<Sequence> deleteById(ID id);

    Sequence* findSequenceById(ID id);
    const Sequence* findSequenceById(ID id) const;

    std::vector<Sequence*> allSequences() const;
    
    //TODO: hack, get rid of this
    std::vector<std::unique_ptr<Sequence>> & notToUseSequences() { return _sequences; }
    private:
    std::vector<std::unique_ptr<Sequence>> _sequences;
    std::vector<Sequence*> _playable; //this is temporary, until list won't move to render plan
};


}