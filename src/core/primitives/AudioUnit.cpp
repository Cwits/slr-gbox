// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/AudioUnit.h"

#include "core/primitives/Parameter.h"
#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioContext.h"

#include "core/utility/basicAudioManipulation.h"

#include "core/BufferManager.h"
#include "logger.h"

namespace slr {

/* 
//use 1024 this as starting point for counting ID's for now, 
because there is some problem with getSourcesForId() and getTargetsForId() ->
result yelds not existing routes(at least for midi...)
*/    
constexpr int FirstUnitId = 0;
static ID uniqueIdCounter = FirstUnitId; 

ID AudioUnit::nextAudioUnitId() {
    return uniqueIdCounter;
}

AudioUnit::AudioUnit(const ClipContainer * initialContainer) :
    _uniqueId(uniqueIdCounter++),
    _volume(ParameterFloat("Volume", 1.0f, 0.f, 1.f)),
    _pan(ParameterFloat("Pan", 0.5f, 0.f, 1.f)),
    _mute(ParameterBool("Mute", 0.f, 0.f, 1.f)),
    _clipContainer(initialContainer)
{

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

Common::Status AudioUnit::setParameter(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp) { 
    AudioUnit * u = ev.setParameter.unit;
    u->_flatParameterList[ev.setParameter.parameterId]->setValue(ev.setParameter.value);
    
    resp.type = FlatEvents::FlatResponse::Type::SetParameter;
    resp.status = Common::Status::Ok;
    resp.commandId = ev.commandId;
    resp.setParameter.unit = u;
    resp.setParameter.parameterId = ev.setParameter.parameterId;
    resp.setParameter.value = ev.setParameter.value;
    return Common::Status::Ok;
}

Common::Status AudioUnit::toggleMidiThru(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp) {
    AudioUnit *u = ev.toggleMidiThru.unit;
    u->_midiThru = ev.toggleMidiThru.newState;

    resp.type = FlatEvents::FlatResponse::Type::ToggleMidiThru;
    resp.status = Common::Status::Ok;
    resp.commandId = ev.commandId;
    resp.toggleMidiThru.unit = u;
    resp.toggleMidiThru.newState = ev.toggleMidiThru.newState;
    return Common::Status::Ok;
}

Common::Status AudioUnit::toggleOmniHwInput(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp) {
    AudioUnit *u = ev.toggleOmniHwInput.unit;
    u->_omniHwInput = ev.toggleOmniHwInput.newState;

    resp.type = FlatEvents::FlatResponse::Type::ToggleOmniHwInput;
    resp.status = Common::Status::Ok;
    resp.commandId = ev.commandId;
    resp.toggleOmniHwInput.unit = u;
    resp.toggleOmniHwInput.newState = ev.toggleOmniHwInput.newState;
    return Common::Status::Ok;
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

Common::Status AudioUnit::swapContainer(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp) {
    resp.swapContainer.oldContainer = ev.swapContainer.unit->_clipContainer;
    
    ev.swapContainer.unit->_clipContainer = ev.swapContainer.container;
    
    resp.type = FlatEvents::FlatResponse::Type::SwapContainer;
    resp.status = Common::Status::Ok;
    resp.swapContainer.unit = ev.swapContainer.unit;
    resp.swapContainer.newContainer = ev.swapContainer.container;
    resp.commandId = ev.commandId;
    return Common::Status::Ok;
}

}