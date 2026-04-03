// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "modules/Sampler/Sampler.h"

#include "core/primitives/AudioContext.h"
#include "core/BufferManager.h"

#include "core/utility/basicAudioManipulation.h"
#include "core/utility/basicMidiManipulation.h"

#include <cassert>
#include <algorithm>

namespace slr {

Sampler::Sampler(const ClipContainer * initContainer) 
    : AudioUnit(initContainer),
    _bufferManager(nullptr),
    _preMix(nullptr),
    _mix(nullptr),
    _file(nullptr),
    _activeVoiceCount(0),
    _voicesHead(0),
    _voicesTail(0),
    _maxVoices(TOTAL_MAX_VOICES)
{
    for(int i=0; i<TOTAL_MAX_VOICES; ++i) {
        _voices[i].clear();
    }
}

Sampler::~Sampler() {

}

bool Sampler::create(BufferManager *man) {
    AudioUnit::create(man);
    _preMix = man->acquireAudioRegular();
    _mix = man->acquireAudioRegular();

    _bufferManager = man;
    return true;
}

bool Sampler::destroy(BufferManager *man) {
    
    man->releaseAudioRegular(_preMix);
    man->releaseAudioRegular(_mix);
    AudioUnit::destroy(man);
    return true;
}

frame_t Sampler::process(const AudioContext &ctx,  const Dependencies &inputs) {
    if(_mute) {
        if(_buffersClear) return ctx.frames;

        clearAudioBuffer((*_preMix)[0], ctx.frames);
        clearAudioBuffer((*_preMix)[1], ctx.frames);
        clearAudioBuffer((*_mix)[0], ctx.frames);
        clearAudioBuffer((*_mix)[1], ctx.frames);
        clearAudioBuffer((*_outputs)[0], ctx.frames);
        clearAudioBuffer((*_outputs)[1], ctx.frames);
        _buffersClear = true;
        return ctx.frames;
    }
    _buffersClear = false;

    
    clearAudioBuffer((*_preMix)[0], ctx.frames);
    clearAudioBuffer((*_preMix)[1], ctx.frames);
    clearAudioBuffer((*_mix)[0], ctx.frames);
    clearAudioBuffer((*_mix)[1], ctx.frames);
    clearAudioBuffer((*_outputs)[0], ctx.frames);
    clearAudioBuffer((*_outputs)[1], ctx.frames);

    // _midiInput->clear();
    // _midiOutput->clear();

    // for(uint32_t i=0; i<inputs.midiDepsCnt; ++i) {
    //     MidiDependencie &mdep = inputs.midi[i];

    //     const MidiBuffer *buf = mdep.external ? getMidiBuffer(ctx, mdep.extId) : mdep.buf;
    //     std::size_t bufSize = buf->size();
    //     for(std::size_t j=0; j<bufSize; ++j) {
    //         const MidiEvent &ev = (*buf)[j];
    //         _midiRecord->push_back(ev);
    //     }
    // }
    
    // sortEventsInMidiBuffer(_midiRecord);



    if(_activeVoiceCount > 0) {
        for(uint8_t v=0; v<_activeVoiceCount; ++v) {

        }
    }

    return ctx.frames;
}

void Sampler::prepareToPlay() {

}

void Sampler::prepareToRecord() {

}

void Sampler::stopPlaying() {

}

void Sampler::stopRecording() {

}

void Sampler::addVoice(uint8_t velocity, uint8_t pitch, frame_t playbackStart, frame_t playbackEnd, frame_t delayBeforePlayback) {
    if(_maxVoices == 1 && _activeVoiceCount != 0) {
        deleteVoice();
    }

    if(_activeVoiceCount >= _maxVoices) return;
    
    Voice &v = _voices[_voicesHead];
    v.velocity = velocity;
    v.pitch = pitch;
    v.playbackPosition = playbackStart;
    v.playbackEnd = playbackEnd;
    v.delayBeforePlayback = delayBeforePlayback;

    if(_voicesHead+1 >= _maxVoices) _voicesHead = 0;
    else _voicesHead++;
    _activeVoiceCount++;
}

void Sampler::deleteVoice() {
    assert(_activeVoiceCount > 0);
    uint8_t toClearId = _voicesTail;
    
    if(_voicesTail+1 >= _maxVoices) _voicesTail = 0;
    else _voicesTail++;
    _activeVoiceCount--;
    _voices[toClearId].clear();
}

void Sampler::clearAllVoices() {
    _activeVoiceCount = 0;
    _voicesHead = 0;
    _voicesTail = 0;
    std::for_each(_voices.begin(), _voices.end(), [](Sampler::Voice &v) {
        v.clear();
    });
}

Common::Status Sampler::assetOpened(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp) {
    ev.assetOpened.sampler->clearAllVoices();
    ev.assetOpened.sampler->_file = ev.assetOpened.file;

    resp.type = FlatEvents::FlatResponse::Type::AssetOpened;
    resp.commandId = ev.commandId;
    resp.status = Common::Status::Ok;
    resp.assetOpened.sampler = ev.assetOpened.sampler;
    resp.assetOpened.file = ev.assetOpened.file;
    return Common::Status::Ok;
}


}