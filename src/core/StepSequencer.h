// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "common/defines.h"
#include "core/primitives/StepEvent.h"
#include "core/Timeline.h" //this is excessive
 
#include <vector>
#include <memory>
#include <array>

namespace slr {

struct AudioUnit;
struct AudioContext;

const int TARGET_COUNT = 8;
const int EVENTS_COUNT = 256;
const int LAYERS_COUNT = 16;
const int STEPS_PER_PAGE = 16;
struct Sequence {
    Sequence(ID uniqueId);
    frame_t tick(const AudioContext &ctx) const;
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
    std::array<frame_t, EVENTS_COUNT> _eventPositions;

    mutable frame_t _referencePoint; //needs to be updated on swap

    void initDefault(const Timeline &tl);
    void recalculateEventPositions(const Timeline &tl, StepDuration newDuration); 
    void initLayer(int layer, int note, int velocity);
    void clearAllLayers();
    void clearLayer(int id);
    void clear();

    StepDuration stepDuration() const { return _stepDuration; }

    private:
    StepDuration _stepDuration;
};

struct StepSequencerEngine {
    StepSequencerEngine();
    ~StepSequencerEngine();

    // void syncSequences();
    void setReferencePoint(const frame_t frame);

    Sequence * createNewSequence();
    // Sequence * cloneSequence(const Sequence *seq);

    // std::vector<StepEvent>* changeSequenceSize(ID seqId, int stepsPerBar, int bars);

    std::unique_ptr<Sequence> deleteById(ID id);

    Sequence* findSequenceById(ID id);
    const Sequence* findSequenceById(ID id) const;

    // std::size_t sequenceCount() const { if(!_playableSequences) return 0; else return _playableSequences->size(); }
    // const std::vector<Sequence*> * sequences() const { return _playableSequences; }
    [[deprecated]]
    std::size_t sequenceCount() const { return _playable.size(); } 
    // [[deprecated]]
    // const std::vector<std::unique_ptr<Sequence>>& sequences() const { return _sequences; }

    std::vector<Sequence*> &playable() { return _playable; }
    const std::vector<Sequence*>& playable() const { return _playable; }

    
    private:
    std::vector<std::unique_ptr<Sequence>> _sequences;
    std::vector<Sequence*> _playable; //this is temporary, until list won't move to render plan
};


}