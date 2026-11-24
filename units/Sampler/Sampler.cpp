// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "units/Sampler/Sampler.h"

#include "core/utility/AudioContext.h"
#include "core/BufferManager.h"

#include "core/primitives/AudioFile.h"
#include "core/primitives/MidiEvent.h"

#include "core/utility/basicAudioManipulation.h"
#include "core/utility/basicMidiManipulation.h"

#include "common/logger.h"

#include <cassert>
#include <algorithm>
#include <limits>

namespace slr {

Sampler::Sampler(const ClipContainer * initContainer, const ID forcedId) 
    : AudioUnit(initContainer, forcedId),
    _bufferManager(nullptr),
    _preMix(nullptr),
    _mix(nullptr),
    _file(nullptr),
    _activeVoiceCount(0),
    _mode(SamplerMode::Classic),
    _rootPitch(ParameterInt("Root Pitch", 36, 0, 127, 1)),
    _maxVoices(ParameterInt("Max Voices", 8, 0, MAX_VOICES, 1)),
    _sampleStart(ParameterFrame("Start", 0, 0, ParameterFrame::typemax(), 1)),
    _sampleEnd(ParameterFrame("End", 0, 0, ParameterFrame::typemax(), 1))
{
    for(int i=0; i<MAX_VOICES; ++i) {
        _voices[i].clear();
    }

    addParameter(&_rootPitch);
    addParameter(&_maxVoices);
    addParameter(&_sampleStart);
    addParameter(&_sampleEnd);
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

frame_t Sampler::process(const AudioContext &ctx,  const Dependencies &inputs) const {
    if(_mute) {
        if(_buffersClear) return ctx.blockSize;

        clearAudioBuffer((*_preMix)[0], ctx.blockSize);
        clearAudioBuffer((*_preMix)[1], ctx.blockSize);
        clearAudioBuffer((*_mix)[0], ctx.blockSize);
        clearAudioBuffer((*_mix)[1], ctx.blockSize);
        clearAudioBuffer((*_outputs)[0], ctx.blockSize);
        clearAudioBuffer((*_outputs)[1], ctx.blockSize);
        _buffersClear = true;
        return ctx.blockSize;
    }
    _buffersClear = false;

    
    clearAudioBuffer((*_preMix)[0], ctx.blockSize);
    clearAudioBuffer((*_preMix)[1], ctx.blockSize);
    clearAudioBuffer((*_mix)[0], ctx.blockSize);
    clearAudioBuffer((*_mix)[1], ctx.blockSize);
    clearAudioBuffer((*_outputs)[0], ctx.blockSize);
    clearAudioBuffer((*_outputs)[1], ctx.blockSize);

    //do not clear midiInput, because there can be some 
    _midiOutput->clear();

    fetchAndSortMidi(ctx, inputs);
    
    if(ctx.playing) {
        //playbackMidi()???
        playbackFiles(ctx, _outputs, _midiInput);
    }

    processIncomingMidi();


    playbackVoices(ctx);
   
    //process volume and stuff
    for(frame_t f=0; f<ctx.blockSize; ++f) {
        (*_outputs)[0][f] *= _volume.value(f);
        (*_outputs)[1][f] *= _volume.value(f);
    }

    // for(frame_t f=0; f<ctx.blockSize; ++f) {
    //     (*_outputs)[0][f] *= _pan.value(f);
    //     (*_outputs)[1][f] *= _pan.value(f);
    // }

    return ctx.blockSize;
}

void Sampler::prepareToPlay() {

}

void Sampler::prepareToRecord() {

}

void Sampler::stopPlaying() {

}

void Sampler::stopRecording() {

}

void Sampler::asset(const AudioFile * file) { 
    _file = file; 
    _sampleEnd = _file->frames();
    _sampleEnd.maximalValue(_file->frames());
}

void Sampler::addVoice(uint8_t velocity, uint8_t pitch, frame_t playbackStart, frame_t playbackEnd, frame_t delayBeforePlayback) const {
    if(_activeVoiceCount >= _maxVoices) {
        return;
    }

    if(_maxVoices == 1 && _activeVoiceCount != 0) {
        for(uint8_t i=0; i<MAX_VOICES; ++i) {
            Voice &v = _voices[i];
            if(!v.active) continue;

            v.velocity = velocity;
            v.pitch = pitch;
            v.playbackPosition = playbackStart;
            v.playbackEnd = playbackEnd;
            v.delayBeforePlayback = delayBeforePlayback;
            v.active = true;
            break;
        }
        return;
    }

    for(uint8_t i=0; i<MAX_VOICES; ++i) {
        Voice &v = _voices[i];
        if(v.active) continue;

        v.velocity = velocity;
        v.pitch = pitch;
        v.playbackPosition = playbackStart;
        v.playbackEnd = playbackEnd;
        v.delayBeforePlayback = delayBeforePlayback;
        v.active = true;
        break;
    }

    _activeVoiceCount++;
}

void Sampler::deleteVoice(uint8_t velocity, uint8_t pitch) const {
    assert(_activeVoiceCount > 0);

    for(uint8_t i=0; i<MAX_VOICES; ++i) {
        Voice &v = _voices[i];
        if(!v.active) continue;
        if(v.velocity != velocity && v.pitch != pitch) continue;

        v.clear();
        _activeVoiceCount--;
    }
}

void Sampler::playbackVoices(const AudioContext &ctx) const {
    if(_activeVoiceCount == 0) return;

    for(uint8_t vv=0; vv<MAX_VOICES; ++vv) {
        Voice &v = _voices[vv];
        if(!v.active) continue;

        //mix audio
        frame_t toPlay = 0;
        // frame_t playbackStart = 0;
        if(v.playbackPosition + ctx.blockSize <= v.playbackEnd) toPlay = ctx.blockSize;
        else {
            toPlay = v.playbackEnd - v.playbackPosition; 
            v.active = false; //last thing
            _activeVoiceCount--;
        }

        const AudioBuffer * raw = _file->data();
        if(raw->channels() == 2) {
            for(frame_t s=0; s<toPlay; ++s) {
                (*_outputs)[0][s] += (*raw)[0][s+v.playbackPosition];
                (*_outputs)[1][s] += (*raw)[1][s+v.playbackPosition];
            }
        } else {
            for(frame_t s=0; s<toPlay; ++s) {
                (*_outputs)[0][s] += (*raw)[0][s+v.playbackPosition];
                (*_outputs)[1][s] += (*raw)[0][s+v.playbackPosition];
            }
        }
        v.playbackPosition += toPlay;
        // LOG_INFO("Voice pos: %lu total: %lu", v.playbackPosition, v.playbackEnd);
    }
}

void Sampler::clearAllVoices() const {
    _activeVoiceCount = 0;
    std::for_each(_voices.begin(), _voices.end(), [](Voice &v) {
        v.clear();
    });
}

void Sampler::processIncomingMidi() const {
    if(_midiInput->size() == 0) return;
    for(std::size_t e=0; e<_midiInput->size(); ++e) {
        const MidiEvent &ev = (*_midiInput)[e];
        switch(ev.type) {
            case(MidiEventType::NoteOn): {
                // addNote();
                if(_mode == SamplerMode::Classic || _mode == SamplerMode::OneShot) {
                    addVoice(
                        ev.velocity, 
                        ev.note,
                        _sampleStart,
                        _sampleEnd,
                        ev.offset
                    );
                } else {
                    //const Slice &s = pickSlice(ev.note);
                    /*addVoice(
                        ev.velocity,
                        ev.note,
                        s.start,
                        s.end,
                        ev.offset
                    );
                    */
                }
            } break;
            case(MidiEventType::NoteOff): {
                //noteOff
                if(_mode == SamplerMode::Classic) {
                    deleteVoice(ev.velocity, ev.note);
                }
            } break;
            case(MidiEventType::CC): {
                //check learned map for some activities
            } break;
        }
    }
}

}