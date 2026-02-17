// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/RtEngine.h"

#include "core/drivers/AudioDriver.h"
#include "core/primitives/AudioBuffer.h"
#include "core/primitives/AudioContext.h"
#include "core/utility/basicAudioManipulation.h"

#include "core/SettingsManager.h"
#include "core/ControlEngine.h"
#include "core/RtEngine.h"
#include "core/RTHandlerTable.h"

#include "core/Project.h"
#include "core/Timeline.h"

#include "core/Metronome.h"

#include "defines.h"

#include <memory>
#include <functional>

namespace slr {

RtEngine::RtEngine() {
    _midiInLocal = new std::vector<RtMidiBuffer>();
    _midiInputMap = new std::vector<RtMidiQueue>();
    _midiOutputMap = new std::vector<RtMidiOutput>();
    _isFirstCallback = true;
}

RtEngine::~RtEngine() {
    shutdown();

    delete _midiInLocal;    
    delete _midiInputMap;
    delete _midiOutputMap;
}

bool RtEngine::init() {
    _snapshotCount = 0;
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
        _state = RtState::ERROR;
    } else {
        _state = RtState::RUN;
    }

    if(_state == RtState::RUN) return true;
    else return false;
}

bool RtEngine::stop() {
    if(_state != RtState::RUN) return false;

    if(_driver->stop()) {
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

    FlatEvents::FlatControl ev;
    while(_inputControl.pop(ev)) {
        _controlSnapshot[_snapshotCount] = ev;
        _snapshotCount++;
    }

    handleControlEvents(_controlSnapshot, framesPassed);

    //midi work
    for(RtMidiBuffer &b : *_midiInLocal) {
        b.buffer->clear();
    }

    for(RtMidiQueue &q : *_midiInputMap) {
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
            out->sendEvent(*peekptr);
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
    const frame_t elapsed = tl.elapsed(framesPassed);
    AudioContext ctx(playing,
                    recording,
                    frames,
                    elapsed,
                    framesPassed,
                    inputs,
                    outputs,
                    tl,
                    _outputControl,
                    _midiInLocal);
    
    const RenderPlan * plan = (_prj->isSolo() ? _prj->soloPlan() : _prj->runPlan());
    for(uint32_t n=0; n<plan->nodesCount; ++n) {
        // const RenderPlan::Node & node = plan->nodes[n];
        plan->nodes[n].target->clearMidiBuffer();
    }

    /* 
    _prj->modEngine->process(ctx, nullptr, 0); -> control events to parameters by target->injectControl(ctrl);

    _prj->stepSequencer->process(ctx, nullptr, 0); -> midi events by target->injectMidi(midi);
    */

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
        Dependencies dummy;
        metro->process(ctx, dummy);
    }

    return frames;
}

const int RtEngine::blockSize() const {
    return _driver->bufferSize();
}

const int RtEngine::channels() const {
    return _driver->outputCount();
}

void RtEngine::handleControlEvents(std::array<FlatEvents::FlatControl, 256> & list, frame_t & framesPassed) {
    for(int i=0; i<_snapshotCount; ++i) {
        const FlatEvents::FlatControl & ev = list[i];
		FlatEvents::FlatResponse resp;
        Status st = RTHandlers::RTTable[static_cast<size_t>(ev.type)](ev, resp);
        
        if(st == Status::Ok)
            _outputControl.push(resp);
    }

    _snapshotCount = 0;
}

void RtEngine::setProject(Project * prj) { 
    prj->timeline().init(_driver->sampleRate(), _driver->bufferSize());
    _prj = prj; 
}

void RtEngine::addRtControl(const FlatEvents::FlatControl &ctl) {
    ControlEngine::rtEngine()->FlatControlEvent(ctl);
}

void RtEngine::addRtResponse(const FlatEvents::FlatResponse & resp) {
    ControlEngine::rtEngine()->FlatResponseEvent(resp);
}

Status RtEngine::updateMidiMaps(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp) {
    RtEngine * engine = ev.updateMidiMaps.engine;
    
    resp.type = FlatEvents::FlatResponse::Type::UpdateMidiMaps;
    resp.status = Status::Ok;
    resp.commandId = ev.commandId;
    resp.updateMidiMaps.oldInput = engine->_midiInputMap;
    resp.updateMidiMaps.oldOutput = engine->_midiOutputMap;
    resp.updateMidiMaps.oldLocal = engine->_midiInLocal;

    engine->_midiInputMap = ev.updateMidiMaps.inputMap;
    engine->_midiOutputMap = ev.updateMidiMaps.outputMap;
    engine->_midiInLocal = ev.updateMidiMaps.localBuffers;

    return Status::Ok;
}

}
