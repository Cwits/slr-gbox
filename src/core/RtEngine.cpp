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

// #include "core/Track.h"
#include "core/Mixer.h"
#include "core/Metronome.h"

#include "defines.h"

#include <memory>
#include <functional>

namespace slr {


// static const FlatEvents::FlatControl _zeroControl = {
//     .type = FlatEvents::FlatControl::Type::Error
// };

RtEngine::RtEngine() {
    
}

RtEngine::~RtEngine() {
    shutdown();
}

bool RtEngine::init() {
    _snapshotCount = 0;
    // _controlSnapshot.fill(_zeroControl);

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

bool RtEngine::start() {
    if(_state == RtState::ERROR) return false;

    if(!_driver->start([this](AudioBuffer * inputs, AudioBuffer * outputs, frame_t frames, frame_t framesPassed) -> frame_t {
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

    // MidiSnapshot = MidiInputs; //depends on how midi messages are acquired - e.g. jack provides input for current block, but if
    // we doing that manually than we need somehow limit incomming. E.g. use double buffering for every input and switch buffers at this stage
    
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
                    _outputControl);
    /* 
    ctx._midiInputs = midiInputs;
    ctx._midiOutputs = midiOutputs;
    */
    
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

    const RenderPlan * plan = (_prj->isSolo() ? _prj->soloPlan() : _prj->runPlan());
    for(uint32_t n=0; n<plan->nodesCount; ++n) {
        const RenderPlan::Node & node = plan->nodes[n];
        node.target->process(ctx, node.deps, node.depsCount);
    }

    //TODO: treat mixer as regular unit...
    _prj->mixer()->process(ctx, plan->mixerDeps, plan->mixerDepsCount);
    
 
    Metronome * metro = _prj->metronome();
    if(ctx.playing) {
        metro->process(ctx, nullptr, 0);
    }

    /*future improvement
        return not only frames count, but
        struct driver countext {
            frame_t frames;
            Dependencies * deps;
            uint32_t depsCount;
        };

        so driver will mix dependencies according to 
        channel maps to appropriate channels
    */

    /*
    TODO: rn mixer pass the finalized data to MainOutputs;
    but it should be smth like
    int mainoutsnum = plan->mainoutscount;
    for(int i=0; i<mainoutsnum; ++i) {
        for(frame_t f=0; f<ctx.frames; ++f) { 
            (*ctx.mainOutputs)[0][f] += (*plan->mainouts)[0][f];
            (*ctx.mainOutputs)[1][f] += (*plan->mainouts)[1][f];
        }
    }
     */

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

    // _controlSnapshot.fill(_zeroControl);
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

}