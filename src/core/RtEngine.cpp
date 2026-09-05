// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/RtEngine.h"

#include "core/drivers/AudioDriver.h"
#include "core/primitives/AudioBuffer.h"
#include "core/utility/AudioContext.h"
#include "core/primitives/RtTask.h"
#include "core/utility/basicAudioManipulation.h"

#include "core/SettingsManager.h"
#include "core/ControlEngine.h"
#include "core/StepSequencer.h"
#include "core/ModulationEngine.h"

#include "core/Project.h"
#include "core/Timeline.h"

#include "core/Metronome.h"

#include "common/defines.h"

#include "common/Profiler.h"

#include <memory>
#include <functional>

namespace slr {

#if RT_PROFILE == 1
const std::string TEST = "Rt Engine"; 
Profiler::ResultQueue * profQueue = nullptr;
#endif

Dependencies metroDeps;

RtEngine::RtEngine() {
    _midiInLocal = new std::vector<RtMidiBuffer>();
    _midiInputMap = new std::vector<RtMidiQueue>();
    _midiOutputMap = new std::vector<RtMidiOutput>();
    _isFirstCallback = true;
#if RT_PROFILE == 1
    profQueue = Profiler::prepare(TEST);
#endif
}

RtEngine::~RtEngine() {
    shutdown();

    delete _midiInLocal;    
    delete _midiInputMap;
    delete _midiOutputMap;
}

bool RtEngine::init() {
    // _snapshotCount = 0;
    _isFirstCallback = true;
    
#if defined(__aarch64__)
    _driver = AudioDriverFactory::create(SettingsManager::getAudioDriver());
#else
    _driver = AudioDriverFactory::create("Dummy Driver");
#endif

    if(!_driver->init(SettingsManager::getSampleRate(),
             SettingsManager::getBlockSize(), 
             DEFAULT_BUFFER_CHANNELS, DEFAULT_BUFFER_CHANNELS)) {
        //error
        _state = RtState::ERROR;
        return false;
    }

    _state = RtState::STOP;
    return true;
}

bool RtEngine::start(std::function<void(frame_t)> anchorLambda) {
    if(_state == RtState::ERROR) return false;

    if(!_driver->start([this, anchorLambda](AudioBuffer * inputs, AudioBuffer * outputs, frame_t frames, frame_t framesPassed) -> frame_t {
        anchorLambda(framesPassed); //lol is that very bad? :/
        return this->processNextBlock(inputs, outputs, frames, framesPassed);
    })) {
        //error
        _state.store(RtState::ERROR, std::memory_order_release);
    } else {
        _state.store(RtState::RUN, std::memory_order_release);
    }

    if(_state == RtState::RUN) return true;
    else return false;
}

bool RtEngine::stop() {
    if(_state != RtState::RUN) return false;

    if(_driver->stop()) {
        _state.store(RtState::STOP, std::memory_order_release);
        return true;
    } else {
        return false;
    }
}

bool RtEngine::shutdown() {
    if(_state != RtState::ERROR) {
        _state = RtState::ERROR;
        // _driver->shutdown(); called automatically, no need to call
    }
    return true;
}

frame_t RtEngine::processNextBlock(AudioBuffer * inputs, AudioBuffer * outputs, frame_t frames, frame_t framesPassed) {
    if(_state == RtState::ERROR) return 0;

    //at this point inputs contains hw input data, and outputs buffers are zeroed out

#if RT_PROFILE == 1
    Profiler::start(profQueue);
#endif
    //handle rt control tasks
    {
        RtTask * task = nullptr;
        while(_rtTasks.pop(task)) {
            task->fn(task->obj);
        }
    }

    //midi work
    for(RtMidiBuffer &b : *_midiInLocal) {
        b.buffer->clear();
    }

    for(RtMidiQueue &q : *_midiInputMap) {
        //MidiBuffer valid for current frame only... bad naming here...
        MidiBuffer *buf = nullptr;
        for(RtMidiBuffer &b : *_midiInLocal) {
            if(q.id == b.id) {
                buf = b.buffer;
                break;
            }
        }
        
        //for echo
        RtMidiOutput * out = nullptr;
        for(RtMidiOutput &o : *_midiOutputMap) {
            if(o.id == q.id) {
                out = &o;
                break;
            }
        }

        const MidiEvent * peekptr;
        while(q.queue->peek(peekptr)) {
            if(peekptr->offset >= framesPassed) {
                MidiEvent ev;
                q.queue->pop(ev);
                ev.offset = ev.offset - framesPassed; //or calculate this in midi input handle, assuming that event will be handled in next frame by default?
                buf->push_back(ev);
                // q.queue->commit_pop();
#if (RT_TRACE == 1)
                LOG_INFO("Midi Ev offset %lu, frames passed %lu", ev.offset, framesPassed);
#endif
            } else {
#if (RT_TRACE == 1)
                LOG_INFO("Still ev left  offset %lu, frames passed %lu", peekptr->offset, framesPassed);
                LOG_ERROR("theoretically Unreachable?");
#endif
                q.queue->commit_pop(); //just skip this
                break;
            }

            //simple echo?
            // out->sendEvent(*peekptr);
        }
    }
    
   /*
    MidiEvent midiev;
    for(RtMidiQueue &q : *_midiInputMap) {
        std::vector<MidiEvent> *v = nullptr;
        for(RtMidiBuffer &b : *_midiInLocal) {
            if(q.id == b.id) {
                v = b.buffer;
                break;
            }
        }

        RtMidiOutput * out = nullptr;
        for(RtMidiOutput &o : *_midiOutputMap) {
            if(o.id == q.id) {
                out = &o;
                break;
            }
        }

        while(q.queue->pop(midiev)) {
            LOG_INFO("Midi Ev offset %lu, frames passed %lu", midiev.offset, framesPassed);
            v->push_back(midiev);
            //test echo
            if(midiev.type == MidiEventType::NoteOn)
                out->sendEvent(midiev);
        }
    }*/
    
    Timeline & tl = _prj->timeline();
    const bool playing = tl.playing();//must be called before elapsed because if prevstate == preparing than we can do 
    const bool recording = tl.recording();
    const bool freewheeling = false;
    const frame_t elapsed = tl.elapsed(framesPassed);
    AudioContext ctx(playing,
                    recording,
                    freewheeling, //freewheeling mode
                    frames,
                    elapsed,
                    framesPassed,
                    inputs,
                    outputs,
                    tl,
                    // _outputControl,
                    _midiInLocal);
    
    const RenderPlan * plan = (_prj->isSolo() ? _prj->soloPlan() : _prj->runPlan());
    for(uint32_t n=0; n<plan->nodesCount; ++n) {
        plan->nodes[n].target->clearMidiInput();
    }

    /* 
    _prj->modEngine->process(ctx, nullptr, 0); -> control events to parameters by target->injectControl(ctrl);

    _prj->stepSequencer->process(ctx, nullptr, 0); -> midi events by target->injectMidi(midi);
    */

    /*
    // if(plan->syncSequences.load(std::memory_order_acquire)) {
    //     for(uint32_t s=0; s<plan->sequenceCount; ++s) {
    //         plan->sequences[s]->setReferencePoint(ctx.totalFrames);
    //     }
    //     plan->syncSequences.store(std::memory_order_relaxed);
    // }

    // if(_prj->stepSequencer()->sequenceCount()) {
    //     const std::vector<Sequence*> * seq = _prj->stepSequencer()->sequences();
    //     for(const Sequence *s : *seq) {
    //         s->tick(ctx);
    //     }
    // }
    */

    if(_prj->modulationEngine()->playable().size()) {
        auto &mod = _prj->modulationEngine()->playable();
        for(auto &m : mod) {
            m->process(ctx);
        }
    }

    if(_prj->stepSequencer()->sequenceCount()) {
        auto &seq = _prj->stepSequencer()->playable();
        for(auto &s : seq) {
            s->tick(ctx);
        }
    }
    
    //for debugging...
#if (RT_TRACE == 1)
    if(ctx.playing) {
        LOG_INFO("playing %s, recording %s, block size %lu, elapsed %lu, total frames passed %lu", 
                    (ctx.playing ? "true" : "false"),
                    (ctx.recording ? "true" : "false"),
                    ctx.frames, 
                    ctx.elapsed,
                    ctx.totalFrames); //2 times elapsed == 0 on dummy driver, but on jack driver - everything fine(only once)
    }
#endif

    for(uint32_t n=0; n<plan->nodesCount; ++n) {
        const RenderPlan::Node & node = plan->nodes[n];
        node.target->process(ctx, node.deps);
    }

    for(uint32_t n=0; n<plan->outputDeps.audioDepsCnt; ++n) {
        const AudioDependencie &ext = plan->outputDeps.audio[n];
        const AudioBuffer * source = ext.external ? ctx.mainInputs : ext.buffer;

        //mix to preFX
        for(int ch=0; ch<32; ++ch) {
            if(ext.channelMap[ch] == -1) continue;

            for(frame_t f=0; f<ctx.frames; ++f) {
                (*ctx.mainOutputs)[ch][f] += (*source)[ext.channelMap[ch]][f];
            }
        }
    }
 
    Metronome * metro = _prj->metronome();
    if(ctx.playing) {
        metro->process(ctx, metroDeps);
    }

#if RT_PROFILE == 1
    Profiler::end(profQueue);
#endif

    return frames;
}

void RtEngine::addTask(RtTask * task) {
    _rtTasks.push(task);
}

const int RtEngine::blockSize() const {
    return _driver->bufferSize();
}

const int RtEngine::channels() const {
    return _driver->outputCount();
}

void RtEngine::setProject(Project * prj) { 
    prj->timeline().init(_driver->sampleRate(), _driver->bufferSize());
    _prj = prj; 
}

void RtEngine::setMidiLocal(std::vector<RtMidiBuffer> *buf) {
    _midiInLocal = buf;
}

void RtEngine::setMidiIn(std::vector<RtMidiQueue> *buf) {
    _midiInputMap = buf;
}

void RtEngine::setMidiOut(std::vector<RtMidiOutput> *buf) {
    _midiOutputMap = buf;
}

void RtEngine::addRtResponse(RtTask * task) {
    ControlEngine::rtEngine()->_rtResponses.push(task);
}

// void RtEngine::processResponses() {
// // SPSCQueue<RtTask*, 256> & resps = _engine->getResponses();
            
//     RtTask * task = nullptr;
//     while(_rtResponses.pop(task)) {
//         task->fn(task->obj);
//     }

// }

}
