// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/AudioUnit.h"

#include "core/primitives/Parameter.h"
#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioContext.h"

#include "core/AudioBufferManager.h"
#include "logger.h"

namespace slr {
    
static ID uniqueIdCounter = 0;

AudioUnit::AudioUnit(AudioUnitType type, bool needsAudioOutputs) : _type(type), _uniqueId(uniqueIdCounter++), _haveAudioOutputs(needsAudioOutputs) {
    addParameter(_volume = new ParameterFloat("Volume", 1.0f, 0.0f, 1.0f));
    addParameter(_pan = new ParameterFloat("Pan", 0.5f, 0.f, 1.f));
    addParameter(_mute = new ParameterBool("Mute", 0.0f, 0.f, 1.f));

    if(needsAudioOutputs) {
        _outputs = AudioBufferManager::acquireRegular();
    }
}

AudioUnit::~AudioUnit() {
    if(_haveAudioOutputs) {
        AudioBufferManager::releaseRegular(_outputs);
    }
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

 
Status AudioUnit::setParameter(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp) { 
    AudioUnit * u = ev.setParameter.unit;
    u->_flatParameterList[ev.setParameter.parameterId]->setValue(ev.setParameter.value);
    
    resp.type = FlatEvents::FlatResponse::Type::SetParameter;
    resp.status = Status::Ok;
    resp.commandId = ev.commandId;
    resp.setParameter.unit = u;
    resp.setParameter.parameterId = ev.setParameter.parameterId;
    resp.setParameter.value = ev.setParameter.value;
    return Status::Ok;
}


Status AudioUnit::appendItem(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp) {
    ev.appendItem.unit->_fileContainer._items->push_back(ev.appendItem.item);
    resp.type = FlatEvents::FlatResponse::Type::AppendItem;
    resp.status = Status::Ok;
    resp.appendItem.unit = ev.appendItem.unit;
    resp.appendItem.item = ev.appendItem.item;
    return Status::Ok;
}

Status AudioUnit::swapContainer(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp) {
    resp.swapContainer.oldContainer = ev.swapContainer.unit->_fileContainer._items;
    
    ev.swapContainer.unit->_fileContainer._items = ev.swapContainer.container;
    
    resp.type = FlatEvents::FlatResponse::Type::SwapContainer;
    resp.status = Status::Ok;
    resp.swapContainer.unit = ev.swapContainer.unit;
    resp.swapContainer.newContainer = ev.swapContainer.container;
    resp.commandId = ev.commandId;
    return Status::Ok;
}

Status AudioUnit::modifyContainerItem(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp) {
    ContainerItem * item = ev.modContainerItem.item;

    item->_startPosition = ev.modContainerItem.startPosition;
    item->_length = ev.modContainerItem.length;
    item->_muted = ev.modContainerItem.muted;

    resp.type = FlatEvents::FlatResponse::Type::ModContainerItem;
    resp.commandId = ev.commandId;
    resp.status = Status::Ok;
    resp.modContainerItem.unit = ev.modContainerItem.unit;
    resp.modContainerItem.item = ev.modContainerItem.item;
    resp.modContainerItem.startPosition = ev.modContainerItem.startPosition;
    resp.modContainerItem.length = ev.modContainerItem.length;
    resp.modContainerItem.muted = ev.modContainerItem.muted;
    return Status::Ok;
}


void AudioUnit::playbackFiles(const AudioContext &ctx, AudioBuffer *buf/*, MidiBuffer *mid */) {
    for(const ContainerItem * item : *(_fileContainer._items)) {
        if(item->_muted) continue;

        if((ctx.elapsed+ctx.frames) <= item->_startPosition) continue;
        if(ctx.elapsed > (item->_startPosition+item->_length)) continue;

        switch(item->_file->type()) {
            case(FileType::Audio): {
                const AudioFile * file = static_cast<AudioFile*>(item->_file);
                const AudioBuffer * data = file->getData();
                int channels = data->channels();

                //TODO: i guess this can be optimized...
                frame_t framesToRead = 0;
                frame_t writePosition = 0;
                frame_t readPosition = 0;
                if(ctx.elapsed < item->_startPosition) { 
                    //beginning
                    framesToRead = ctx.frames - (item->_startPosition - ctx.elapsed);
                    writePosition = item->_startPosition - ctx.elapsed;
                    readPosition = 0;
                } else if(ctx.elapsed + ctx.frames > item->_startPosition + item->_length) {
                    //end
                    framesToRead = (item->_startPosition + item->_length) - ctx.elapsed;
                    writePosition = 0;
                    readPosition = ctx.elapsed - item->_startPosition;
                } else {
                    //middle
                    framesToRead = ctx.frames;
                    writePosition = 0;
                    readPosition = ctx.elapsed - item->_startPosition;
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

}