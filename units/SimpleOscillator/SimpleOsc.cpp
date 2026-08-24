// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "units/SimpleOscillator/SimpleOsc.h"

#include "core/primitives/AudioContext.h"
#include "core/primitives/RenderPlan.h"

#include "core/SettingsManager.h"

#include "core/utility/basicAudioManipulation.h"

#include <cmath>

namespace slr {

SimpleOsc::SimpleOsc(const ClipContainer *initContainer, const ID forcedId) : AudioUnit(initContainer, forcedId) {
    _phase = 0.0f;
    _deltaTime = 1.0f/(float)SettingsManager::getSampleRate();
    _time = 0.0f;
    _volume = 0.7f;
}

SimpleOsc::~SimpleOsc() {

}

frame_t SimpleOsc::process(const AudioContext &ctx, const Dependencies &inputs) {
    if(isMuted(ctx)) return ctx.frames;

    _midiOutput->clear();

    fetchAndSortMidi(ctx, inputs);


    for(std::size_t i=0; i<_midiInput->size(); ++i) {
        const MidiEvent &ev = (*_midiInput)[i];

        //TODO: push to midi frame buffer
        //see TODO.txt:41
            
        {
        switch(ev.type) {
            case(MidiEventType::NoteOn): {
                if(_activeVoices >= 15) continue;

                voice &v = _voices[_activeVoices];
                v.framesDelay = ev.offset;
                v.pitch = ev.note;
                v.velocity = ev.velocity;
                v._time = 0;

                _activeVoices++;
            } break;
            case(MidiEventType::NoteOff): {
                if(_activeVoices > 0)
                    _activeVoices--;
            } break;
        }
        }
    }
/*
    //parse midi
    for(int i=0; i<inputs.midiDepsCnt; ++i) {
        MidiDependencie &dep = inputs.midi[i];

        const MidiBuffer *buf = dep.external ? 
            getMidiBuffer(ctx, dep.extId) :
            dep.buf;

        for(int j=0; j<buf->size(); ++j) {
            const MidiEvent &ev = (*buf)[j];

            //TODO: push to midi frame buffer
            //see TODO.txt:41
            
            {
            switch(ev.type) {
                case(MidiEventType::NoteOn): {
                    if(_activeVoices >= 15) continue;

                    voice &v = _voices[_activeVoices];
                    v.framesDelay = 0;
                    v.pitch = ev.note;
                    v.velocity = ev.velocity;
                    v._time = 0;

                    _activeVoices++;
                } break;
                case(MidiEventType::NoteOff): {
                    if(_activeVoices > 0)
                        _activeVoices--;
                } break;
            }
            }

            if(_midiThru) _midiOutput->push_back(ev);
        }
    }

    */

    //TODO: gather from injected

    //TODO sort midi events

    //TODO: pass to midi thru

    clearAudioBuffer((*_outputs)[0], ctx.frames);
    clearAudioBuffer((*_outputs)[1], ctx.frames);

    if(ctx.playing) {
        playbackFiles(ctx, _outputs, _midiInput);
    }

    //generate
    for(int i=0; i<_activeVoices; ++i) {
        voice &v = _voices[i];
        
        for(frame_t f=0; f<ctx.frames; ++f) {
            if(v.framesDelay != 0) v.framesDelay--;
            else {
                float freq = std::pow(2, (float)(v.pitch-24)/12) * 440.0f;

                sample_t res = std::sin(2*M_PI*freq*v._time + _phase) / _activeVoices;
                v._time += _deltaTime;

                (*_outputs)[0][f] += res;
                (*_outputs)[1][f] += res;
            }
        }
    }

    //process volume
    const float volume = _volume;
    for(frame_t f=0; f<ctx.frames; ++f) {
        (*_outputs)[0][f] *= volume;
        (*_outputs)[1][f] *= volume;
    }

    return ctx.frames;
}

void SimpleOsc::prepareToPlay() {

}

void SimpleOsc::prepareToRecord() {

}

void SimpleOsc::stopPlaying() {
    _activeVoices = 0;
}

void SimpleOsc::stopRecording() {

}


}