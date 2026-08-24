// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/StepSequencer.h"
#include "core/primitives/AudioContext.h"
#include "core/primitives/AudioUnit.h"

#include "core/Project.h"
#include "core/Timeline.h"

#include "logger.h"

#include <cmath>
#include <cstdlib>

namespace slr {

ID sequenceIdCounter = 1;


StepSequencerEngine::StepSequencerEngine()
{
    _playable.reserve(99);
    
}

StepSequencerEngine::~StepSequencerEngine() {

}

/* 
    must be called on when freewheeling mode is set, 
    because if we use play-stop than everything is fine, read elapsed,
    but in freewheeling we need some kind of reference point to be able to determine
    at what position now things are

    set in total driver frame
*/
void StepSequencerEngine::setReferencePoint(const frame_t frame) {
    for(std::unique_ptr<Sequence> &s : _sequences) {
        s->_referencePoint = frame;
    }
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
        // return std::make_pair<std::unique_ptr<Sequence>, EventHolder>(std::unique_ptr<Sequence>(), EventHolder{});
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

// std::vector<StepEvent>* StepSequencerEngine::changeSequenceSize(ID seqId, int stepsPerBar, int bars) {
//     EventHolder * tmp = nullptr;
//     for(EventHolder &h : _holders) {
//         if(h._sequenceId == seqId) {
//             tmp = &h;
//         }
//     }

//     EventHolder &holder = *tmp;
//     Sequence * seq = findSequenceById(seqId);
//     //assume that such seq exists ofc...

//     std::vector<StepEvent> *editable = nullptr;
//     const std::vector<StepEvent> *other = nullptr;
//     if(seq->_events == holder._holder1.get()) {
//         editable = holder._holder2.get();
//         other = holder._holder1.get();
//     } else {
//         editable = holder._holder1.get();
//         other = holder._holder2.get();
//     }

//     editable->resize(stepsPerBar*bars);
//     for(std::size_t i=0; i<editable->size(); ++i) {
//         if(i < other->size()) {
//             (*editable)[i] = (*other)[i];
//         } else {
//             (*editable)[i].clear();
//         }
//     }

//     return editable;
// }

Sequence::Sequence(ID uniqueId) : 
    _uniqueId(uniqueId),
    _stepCount(16),
    _stepDuration(StepDuration::d16),
    _referencePoint(0)
{
    for(int i=0; i<TARGET_COUNT; ++i) _targets[i] = nullptr;
    for(int i=0; i<EVENTS_COUNT; ++i) _eventPositions[i] = 0;
}

void Sequence::initDefault(const Timeline &tl) {
    clearAllLayers();
    _stepCount = 16;
    _referencePoint = 0;

    for(int i=0; i<LAYERS_COUNT; ++i) initLayer(i, 36+i, 120);
    for(int i=0; i<TARGET_COUNT; ++i) _targets[i] = nullptr;
    for(int i=0; i<EVENTS_COUNT; ++i) _eventPositions[i] = 0;
    recalculateEventPositions(tl, StepDuration::d16);
}

frame_t Sequence::tick(const AudioContext &ctx) const {
    if(!ctx.playing && !ctx.freewheeling) return ctx.frames;
    
    bool hasTargets = false;
    for(int i=0; i<TARGET_COUNT; ++i) {
        if(_targets[i] != nullptr) { hasTargets = true; break; }
    }

    if(!hasTargets) return ctx.frames;

    frame_t framesPerStep = ctx.timeline.framesInStep(_stepDuration);
    frame_t frameHalf = framesPerStep/2;
    frame_t loopLength = framesPerStep * _stepCount;
    
    frame_t positionWithinLoop = 0;
    if(ctx.freewheeling) {
        positionWithinLoop = (ctx.totalFrames - _referencePoint) % loopLength;
    } else {
        positionWithinLoop = ctx.elapsed % loopLength;
    }

    //we getting value [0, _stepCount) 
    unsigned int step = positionWithinLoop / framesPerStep; 
    bool toTrigger = false;
    //some processing to figure out step and whether it should be triggered

    //is this one right? problem is that on step 0 we have to be in frame 0, but otherwise we have to check next step
    // if(step != 0) step++;
    if(positionWithinLoop != 0) {
        step++;
    } 

    if(_eventPositions[step] >= positionWithinLoop && _eventPositions[step] <= positionWithinLoop+ctx.frames) {
        toTrigger = true;
    }

    
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
    for(int i=0; i<LAYERS_COUNT; ++i) {
        const Layer &l = _layers[i];
        if(!l._active) continue;
        if(l._mute) continue;
        
        //somehow check that previous event was on long enough
        if(l._eventTriggered) {
            frame_t elapsedSince = 0;
            const frame_t &evPosition = _eventPositions[l._lastTriggeredEvent];
            if(positionWithinLoop > evPosition) 
                elapsedSince = positionWithinLoop - evPosition;
            else 
                elapsedSince = (loopLength - evPosition) + positionWithinLoop;

            if(elapsedSince >= frameHalf-ctx.frames) {
                //send noteOff for prev active event
                MidiEvent ev = l.findLastActive(l._lastTriggeredEvent);
                ev.note = l._note;
                ev.velocity = 0;
                ev.type = MidiEventType::NoteOff;

                if(!l._target) {
                    for(int i=0; i<TARGET_COUNT; ++i) {
                        if(_targets[i]) _targets[i]->injectMidi(ev);
                    }
                } else {
                    l._target->injectMidi(ev);
                }

                // LOG_INFO("Step off %d on at %lu, with delay %lu, total %lu", l._lastTriggeredEvent, evPosition, 0, positionWithinLoop);
                l._eventTriggered = false;
            }
        }

        if(l._events[step]._enabled && toTrigger) {
            MidiEvent ev = l._events[step]._event;
            ev.note = l._note;
            ev.velocity = l._velocity;
            //do some magic with ev.offset e.g. swing and/or delay
            frame_t delay = _eventPositions[step] - positionWithinLoop;
            ev.offset = delay;

            if(!l._target) {
                for(int i=0; i<TARGET_COUNT; ++i) {
                    if(_targets[i]) _targets[i]->injectMidi(ev);
                }
            } else {
                l._target->injectMidi(ev);
            }

            // LOG_INFO("Step on %d on at %lu, with delay %lu, total %lu", step, _eventPositions[step], delay, positionWithinLoop);
            l._lastTriggeredEvent = step;
            l._eventTriggered = true;
        }
    }

    if(ctx.freewheeling) {
        if(ctx.totalFrames+64 - _referencePoint >= loopLength) {
            _referencePoint = ctx.totalFrames+64;
        }
    }
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

void Sequence::recalculateEventPositions(const Timeline &tl, StepDuration newDuration) {
    _stepDuration = newDuration;
    frame_t framesPerStep = tl.framesInStep(newDuration);
    float frameFraction = tl.framesInStepFraction(newDuration);
    for(int i=0; i<EVENTS_COUNT; ++i)
        _eventPositions[i] = (i*framesPerStep) + std::floor(i*frameFraction);
}

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
    for(int i=0; i<EVENTS_COUNT; ++i) _eventPositions[i] = 0;
}


}