// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/StepSequencer.h"
#include "core/utility/AudioContext.h"
#include "core/primitives/AudioUnit.h"

#include "core/Project.h"
#include "core/Timeline.h"

#include "common/logger.h"
#include "common/Math.h"
#include "common/core_config.h"

#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <limits>

namespace slr {

ID sequenceIdCounter = 1;


StepSequencerEngine::StepSequencerEngine()
{
    _playable.reserve(99);
    
}

StepSequencerEngine::~StepSequencerEngine() {

}

Sequence * StepSequencerEngine::createNewSequence() {
   _sequences.push_back(std::make_unique<Sequence>(sequenceIdCounter));
    sequenceIdCounter++;

    Sequence * ret = _sequences.back().get();

    return ret;
}

// Sequence * StepSequencerEngine::cloneSequence(const Sequence *seq) {
//     _sequences.push_back(std::make_unique<Sequence>(*seq));
//     return _sequences.back().get();
// }

std::unique_ptr<Sequence> StepSequencerEngine::deleteById(ID id) {
    auto it = std::find_if(
        _sequences.begin(),
        _sequences.end(),
        [id](const auto &s) {
            return s->_uniqueId == id;
        }
    );

    if(it == _sequences.end()) {
        LOG_ERROR("Failed to find sequence with id %lu", id);
        return std::unique_ptr<Sequence>();
    }

    std::unique_ptr<Sequence> ret = std::move(*it);
    _sequences.erase(it);

    return ret;
}

const Sequence* StepSequencerEngine::findSequenceById(ID id) const {
    auto it = std::find_if(
        _sequences.begin(),
        _sequences.end(),
        [id](const auto &s) {
            return s->_uniqueId == id;
        }
    );

    if(it == _sequences.end()) {
        LOG_ERROR("Failed to find sequence with id %lu", id);
        return nullptr;
    }

    return (*it).get();
}

Sequence * StepSequencerEngine::findSequenceById(ID id) {
    auto it = std::find_if(
        _sequences.begin(),
        _sequences.end(),
        [id](const auto &s) {
            return s->_uniqueId == id;
        }
    );

    if(it == _sequences.end()) {
        LOG_ERROR("Failed to find sequence with id %lu", id);
        return nullptr;
    }

    return (*it).get();
}

Sequence::Sequence(ID uniqueId) : 
    _uniqueId(uniqueId),
    _stepCount(16),
    _stepDuration(StepDuration::d16)
{
    for(int i=0; i<TARGET_COUNT; ++i) _targets[i] = nullptr;
    // for(int i=0; i<EVENTS_COUNT; ++i) _eventPositions[i] = 0;
}

void Sequence::initDefault(const Timeline &tl) {
    clearAllLayers();
    _stepCount = 16;

    for(int i=0; i<LAYERS_COUNT; ++i) initLayer(i, 36+i, 120);
    for(int i=0; i<TARGET_COUNT; ++i) _targets[i] = nullptr;
    // for(int i=0; i<EVENTS_COUNT; ++i) _eventPositions[i] = 0;
    // recalculateEventPositions(tl, StepDuration::d16);
    _framesTillNextStep = 0;
    _framesTillNoteOff = std::numeric_limits<frame_t>::max();
    // _noteTriggered = false;
}

void Sequence::prepareToPlay() const {
    _framesTillNextStep = 0;
    _framesTillNoteOff = std::numeric_limits<frame_t>::max();
    // _noteTriggered = false;
}

void Sequence::stopAfterPlay() const {
    _framesTillNextStep = 0;
    _framesTillNoteOff = std::numeric_limits<frame_t>::max();
    
    for(int i=0; i<LAYERS_COUNT; ++i) {
        const Layer &l = _layers[i];
            
        if(!l._active) continue;
        if(l._mute) continue;
            
        if(!l._eventTriggered) continue;

        //send note off to all targets
        MidiEvent ev = l._events[l._lastTriggeredEvent]._event;
        ev.type = MidiEventType::NoteOff;
            
        if(!l._target) {
            for(int i=0; i<TARGET_COUNT; ++i) {
                if(_targets[i]) _targets[i]->injectMidi(ev);
            }
        } else {
            l._target->injectMidi(ev);
        }
    }
}

frame_t Sequence::process(const AudioContext &ctx) const {
    if(!ctx.playing) return ctx.frames;
    
    bool hasTargets = false;
    for(int i=0; i<TARGET_COUNT; ++i) {
        if(_targets[i] != nullptr) { hasTargets = true; break; }
    }

    if(!hasTargets) return ctx.frames;

    frame_t framesPerStep = ctx.timeline.framesInStep(_stepDuration);
    float frameFraction = ctx.timeline.framesInStepFraction(_stepDuration);
    frame_t fraction = sMath::floor(frameFraction * _stepCount);
    frame_t loopLength = framesPerStep * _stepCount + fraction;
    
    frame_t positionWithinLoop = 0;
    positionWithinLoop = ctx.elapsed % loopLength;
    
    /* 
        TODO: sooo... the events in StepEvent must remain clean(note = 0. velocity = 0).
        when ticking do check
        MidiEvent ev = findLastActive(...)
        if(ev.note == 0) ev.note = l._note;
        if(ev.velocity == 0) ev.velocity = l._velocity;
        ... offset??

        so, the layer's note and velocity remains primarily source of data, but if event 
        have something - than it overrides.
        
    */

    float fps = framesPerStep + frameFraction;
    
    if(_framesTillNextStep < ctx.frames) {
        _framesTillNoteOff = framesPerStep/2;
        
        frame_t delay = _framesTillNextStep;


        int eventStep = (positionWithinLoop + ctx.frames) / framesPerStep;
        frame_t tmp = fps * (eventStep+1);
        _framesTillNextStep = tmp - positionWithinLoop;

#if STEP_SEQUENCER_TRACE == 1
        LOG_INFO("Step %d: note on: position %lu, with delay %lu, current %lu, next: %lu", 
                    eventStep, 
                    positionWithinLoop, 
                    delay, 
                    positionWithinLoop+delay,
                    positionWithinLoop+_framesTillNextStep);
#endif

        //possible NoteOn Event
        //check if layer have event enabled
        int step = eventStep % _stepCount;
        for(int i=0; i<LAYERS_COUNT; ++i) {
            const Layer &l = _layers[i];
            if(!l._active) continue;
            if(l._mute) continue;
            
            if(!l._events[step]._enabled) continue;

            MidiEvent ev = l._events[step]._event;
            ev.note = l._note;
            ev.velocity = l._velocity;
            ev.offset = delay;
            ev.type = MidiEventType::NoteOn;

            if(!l._target) {
                for(int i=0; i<TARGET_COUNT; ++i) {
                    if(_targets[i]) _targets[i]->injectMidi(ev);
                }
            } else {
                l._target->injectMidi(ev);
            }

            l._lastTriggeredEvent = step;
            l._eventTriggered = true;
        }
    }

    if(_framesTillNoteOff < ctx.frames) {
        int step = positionWithinLoop / framesPerStep;
        frame_t frac = sMath::floor(frameFraction * ((step%_stepCount)+1));
        frame_t delay = _framesTillNoteOff;

#if STEP_SEQUENCER_TRACE == 1
            // LOG_INFO("Step %d: note off: position %lu, with delay %lu, leftover: 0, total %lu", 
            //             (positionWithinLoop + ctx.frames) / framesPerStep, 
            //             positionWithinLoop, 
            //             delay, 
            //             positionWithinLoop+delay);
#endif

        //possible NoteOff Event
        //check if layer had event triggered
        for(int i=0; i<LAYERS_COUNT; ++i) {
            const Layer &l = _layers[i];
            if(!l._active) continue;
            if(l._mute) continue;

            if(!l._eventTriggered) continue;

            MidiEvent ev = l._events[l._lastTriggeredEvent]._event;
            ev.note = l._note;
            ev.velocity = l._velocity;
            ev.offset = delay;
            ev.type = MidiEventType::NoteOff;

            if(!l._target) {
                for(int i=0; i<TARGET_COUNT; ++i) {
                    if(_targets[i]) _targets[i]->injectMidi(ev);
                }
            } else {
                l._target->injectMidi(ev);
            }

            l._lastTriggeredEvent = 0;
            l._eventTriggered = false;

        }

        _framesTillNoteOff = std::numeric_limits<frame_t>::max();
    }

    _framesTillNextStep -= ctx.frames;
    _framesTillNoteOff -= ctx.frames;

   return ctx.frames;
}

void Sequence::Layer::initAllTo(MidiEvent ev) {
    for(int i=0; i<EVENTS_COUNT; ++i) _events[i]._event = ev;
}

void Sequence::Layer::clear() {
    _target = nullptr;
    for(int i=0; i<EVENTS_COUNT; ++i) _events[i].clear();
    _note = 0;
    _velocity = 0;
    _mute = false;
    _active = false;
    _lastTriggeredEvent = 0;
    _eventTriggered = false;
}

const MidiEvent Sequence::Layer::findLastActive(unsigned int current) const {
    for(int i=current-1; i!=current; --i) {
        if(i < 0) i+= EVENTS_COUNT;
        if(_events[i]._enabled) return _events[i]._event;
    }
    return MidiEvent{};
}

// void Sequence::recalculateEventPositions(const Timeline &tl, StepDuration newDuration) {
//     _stepDuration = newDuration;
//     frame_t framesPerStep = tl.framesInStep(newDuration);
//     float frameFraction = tl.framesInStepFraction(newDuration);
//     for(int i=0; i<EVENTS_COUNT; ++i)
//         _eventPositions[i] = (i*framesPerStep) + std::floor(i*frameFraction);
// }

void Sequence::initLayer(int layer, int note, int velocity) {
    if(layer >= LAYERS_COUNT) return;
    MidiEvent ev;
    ev.clear();
    ev.channel = 0;
    ev.note = note;
    ev.offset = 0;
    ev.type = MidiEventType::NoteOn;
    ev.velocity = velocity;

    Layer &l = _layers[layer];
    l.clear();
    l.initAllTo(ev);
    l._note = note;
    l._velocity = velocity;
    l._active = false;
}

void Sequence::clearAllLayers() {
    for(int i=0; i<LAYERS_COUNT; ++i) _layers[i].clear();
}

void Sequence::clearLayer(int id) {
    if(id >= LAYERS_COUNT) return;
    _layers[id].clear();
}

void Sequence::clear() {
    clearAllLayers();
    
    _framesTillNextStep = 0;
    _framesTillNoteOff = std::numeric_limits<frame_t>::max();
    // _noteTriggered = false;
    // for(int i=0; i<EVENTS_COUNT; ++i) _eventPositions[i] = 0;
}


std::vector<Sequence*> StepSequencerEngine::allSequences() const {
    std::vector<Sequence*> ret;
    ret.reserve(_sequences.size());
    for(auto &seq : _sequences) {
        ret.push_back(seq.get());
    }
    return ret;
}

}