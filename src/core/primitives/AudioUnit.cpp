// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/AudioUnit.h"

#include "core/primitives/Parameter.h"
#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioContext.h"

#include "core/utility/basicAudioManipulation.h"

#include "core/BufferManager.h"
#include "logger.h"

#include <cmath>

namespace slr {

// constexpr int FirstUnitId = 1;
static ID uniqueIdCounter = 0; 

ID AudioUnit::nextAudioUnitId() {
    return uniqueIdCounter;
}

AudioUnit::AudioUnit(const ClipContainer * initialContainer, ID id) :
    _uniqueId(id),
    _volume(ParameterFloat("Volume", 1.0f, 0.f, 1.f)),
    _pan(ParameterFloat("Pan", 0.5f, 0.f, 1.f)),
    _mute(ParameterBool("Mute", 0.f, 0.f, 1.f)),
    _clipContainer(initialContainer)
{
    //to ensure that counter never be less than last id(usefull when loading project) - update value if ID is forced
    ID testres = std::max(_uniqueId, uniqueIdCounter);
    if(testres == uniqueIdCounter) uniqueIdCounter = testres+1;
    else uniqueIdCounter = testres;
}

AudioUnit::~AudioUnit() {
    // AudioBufferManager::releaseRegular(_outputs);
}

bool AudioUnit::create(BufferManager *man) {
    addParameter(&_volume);
    addParameter(&_pan);
    addParameter(&_mute);
    _outputs = man->acquireAudioRegular();
    _midiInput = man->acquireMidiRegular();
    _midiOutput = man->acquireMidiRegular();
    return true;
}

bool AudioUnit::destroy(BufferManager *man) {
    man->releaseAudioRegular(_outputs);
    man->releaseMidiRegular(_midiInput);
    man->releaseMidiRegular(_midiOutput);
    return true;
}

void AudioUnit::addParameter(ParameterBase * base) {
    _flatParameterList.add(base);
}

bool AudioUnit::hasParameterWithId(ID parameterId) {
    const std::size_t count = _flatParameterList.count();
    for(std::size_t i=0; i<count; ++i) {
        if(_flatParameterList[i]->id() == parameterId)
            return true;
    }

    return false;
}

bool AudioUnit::isMuted(const AudioContext &ctx) {
    if(_mute) {
        if(!_buffersClear) {
            _buffersClear = true;
            clearAudioBuffer((*_outputs)[0], ctx.frames);
            clearAudioBuffer((*_outputs)[1], ctx.frames);
        }
        return true;
    }

    _buffersClear = false;
    return false;
}

void AudioUnit::applyMidiEvents(MidiBuffer *buf) {

}

void AudioUnit::playbackFiles(const AudioContext &ctx, AudioBuffer *buf, MidiBuffer *mid) {
    if(!_clipContainer) return; //because container created only when some files is loaded

    for(const ClipItem * const item : *_clipContainer) {
        if(item->isMuted()) continue;

        if((ctx.elapsed+ctx.frames) <= item->startPosition()) continue;
        if(ctx.elapsed > (item->startPosition()+item->length())) continue;

        switch(item->_file->type()) {
            case(FileType::Audio): {
                const AudioFile * const file = static_cast<const AudioFile* const>(item->_file);
                const AudioBuffer * data = file->getData();
                int channels = data->channels();

                //TODO: i guess this can be optimized...
                frame_t framesToRead = 0;
                frame_t writePosition = 0;
                frame_t readPosition = 0;
                if(ctx.elapsed < item->startPosition()) { 
                    //beginning
                    framesToRead = ctx.frames - (item->startPosition() - ctx.elapsed);
                    writePosition = item->startPosition() - ctx.elapsed;
                    readPosition = 0;
                } else if(ctx.elapsed + ctx.frames > item->startPosition() + item->length()) {
                    //end
                    framesToRead = (item->startPosition() + item->length()) - ctx.elapsed;
                    writePosition = 0;
                    readPosition = ctx.elapsed - item->startPosition();
                } else {
                    //middle
                    framesToRead = ctx.frames;
                    writePosition = 0;
                    readPosition = ctx.elapsed - item->startPosition();
                }

                // {
                //     LOG_INFO("elapsed: %lu, toRead: %lu, write: %lu, read: %lu, total: %lu", 
                //                 ctx.elapsed,
                //                 framesToRead,
                //                 writePosition,
                //                 readPosition,
                //                 data->bufferSize());
                // }

                if(channels == 1) {
                    for(frame_t f=0; f<framesToRead; ++f) {
                        (*buf)[0][writePosition+f] += (*data)[0][readPosition+f];
                        (*buf)[1][writePosition+f] += (*data)[0][readPosition+f];
                    }
                } else if(channels == 2) {
                    for(frame_t f=0; f<framesToRead; ++f) {
                        // for(int ch=0; ch<channels; ++ch) {
                        //     (*buf)[ch][writePosition+f] += (*data)[ch][readPosition+f];
                        // }
                        (*buf)[0][writePosition+f] += (*data)[0][readPosition+f];
                        (*buf)[1][writePosition+f] += (*data)[1][readPosition+f];
                    }
                }

            } break;
            case(FileType::Midi): {

            } break;
            case(FileType::AudioPeak):
            default: {
                LOG_ERROR("Wrong file type");
            } break;
        }
    }
}

void AudioUnit::clearMidiBuffer() { _midiInput->clear(); }

}