// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modules/Track/Track.h"

#include "core/primitives/AudioContext.h"
#include "core/primitives/Parameter.h"
#include "core/primitives/AudioFile.h"
#include "core/primitives/RenderPlan.h"

#include "core/AudioBufferManager.h"
#include "core/FileWorker.h"
#include "core/FileTasks.h"
#include "core/SettingsManager.h"
#include "core/ControlEngine.h"
#include "core/RtEngine.h"

#include "core/utility/basicAudioManipulation.h"
#include "core/utility/helper.h"

#include "logger.h"
#include "defines.h"

namespace slr {


Track::Track() : AudioUnit(AudioUnitType::Track) {
    _sendToMixer = true;

    _recInt = AudioBufferManager::acquireRegular();
    _recExt = AudioBufferManager::acquireRegular();
    _preFX = AudioBufferManager::acquireRegular();
    _postFX = AudioBufferManager::acquireRegular();
    _postPan = AudioBufferManager::acquireRegular();  

    _record = false;
    _recordSource = RecordSource::Audio;
    _recordTarget = nullptr;
}

Track::~Track() {
    AudioBufferManager::releaseRegular(_preFX);
    AudioBufferManager::releaseRegular(_postFX);
    AudioBufferManager::releaseRegular(_postPan);

    if(_recordTarget) {
        FileWorker * fw = ControlEngine::fileWorker();
        _recordTarget->release(fw);
        delete _recordTarget; 
        _recordTarget = nullptr;
    }
}

frame_t Track::process(const AudioContext &ctx,  const Dependencies * const inputs, const uint32_t inputsCount) {
    /* 
    handle spsc control queue events(because some may be injected via mod engine or automations)
    
    */
    
    //TODO: must ensure that buffers are clear - i guess introduce some flag _buffersClear(as in metronome)
    if(*_mute) {
        if(!_buffersClear) {
            _buffersClear = true;
            
            clearAudioBuffer((*_recInt)[0], ctx.frames);
            clearAudioBuffer((*_recInt)[1], ctx.frames);
            clearAudioBuffer((*_recExt)[0], ctx.frames);
            clearAudioBuffer((*_recExt)[1], ctx.frames);

            clearAudioBuffer((*_preFX)[0], ctx.frames);
            clearAudioBuffer((*_preFX)[1], ctx.frames);

            clearAudioBuffer((*_postFX)[0], ctx.frames);
            clearAudioBuffer((*_postFX)[1], ctx.frames);
                
            clearAudioBuffer((*_postPan)[0], ctx.frames);
            clearAudioBuffer((*_postPan)[1], ctx.frames);
        }
        return ctx.frames;
    }
    
    _buffersClear = false;
#if (DEFAULT_BUFFER_CHANNELS == 2)
    clearAudioBuffer((*_recInt)[0], ctx.frames);
    clearAudioBuffer((*_recInt)[1], ctx.frames);
    clearAudioBuffer((*_recExt)[0], ctx.frames);
    clearAudioBuffer((*_recExt)[1], ctx.frames);

    clearAudioBuffer((*_preFX)[0], ctx.frames);
    clearAudioBuffer((*_preFX)[1], ctx.frames);

    clearAudioBuffer((*_postFX)[0], ctx.frames);
    clearAudioBuffer((*_postFX)[1], ctx.frames);
        
    clearAudioBuffer((*_postPan)[0], ctx.frames);
    clearAudioBuffer((*_postPan)[1], ctx.frames);
#else
    uint8_t channels = _preFX->channels(); 
    for(uint8_t i=0; i<channels; ++i) {
        clearAudioBuffer((*_preFX)[i], ctx.frames);
    }
    channels = _postFX->channels();
    for(uint8_t i=0; i<channels; ++i) {
        clearAudioBuffer((*_postFX)[i], ctx.frames);
    }
    channels = _postPan->channels();
    for(uint8_t i=0; i<channels; ++i) {
        clearAudioBuffer((*_postPan)[i], ctx.frames);
    }
#endif

    if(_record) {
        //_preFX += _inputs
        
        for(int i=0; i<inputsCount; ++i) {
            const Dependencies &ext = inputs[i];

            AudioBuffer * target = ext.external ? _recExt : _recInt;
            const AudioBuffer * source = ext.external ? ctx.mainInputs : ext.buffer;

            //to record
            for(int ch=0; ch<32; ++ch) {
                if(ext.channelMap[ch] == -1) continue;

                for(frame_t f=0; f<ctx.frames; ++f) {
                    (*target)[ch][f] += (*source)[ext.channelMap[ch]][f];
                }
            }

            //mix to preFX
            for(int ch=0; ch<32; ++ch) {
                if(ext.channelMap[ch] == -1) continue;

                for(frame_t f=0; f<ctx.frames; ++f) {
                    (*_preFX)[ch][f] += (*source)[ext.channelMap[ch]][f];
                }
            }
        }

        if(ctx.playing && ctx.recording) {
            if(_recordSource == RecordSource::Audio) {
                _recordTarget->writeData(_recInt, ctx.frames, 2, false); //don't compensate
                _recordTarget->writeData(_recExt, ctx.frames, 2, true); //compensate latency
                _recordTarget->incrementCounter(ctx.frames);
            } else {
                //_recordTarget->writeData(midi, frames);
            }
        } 
    }

    if(ctx.playing) {
        AudioUnit::playbackFiles(ctx, _preFX);
    }
    
    /* preFX buffers completed */
    
    /* process FX chain */
    /*
    int size = _FXChain.length();
    if(size == 0) {
        copy preFX buffers to postFX buffers
    } else {
        process FX chain
        use _preFX as tmp buffer?
        at the end there should be _postFX buffer filled with processed audio
    }
    */

   copyAudioBuffer((*_preFX)[0], (*_postFX)[0], ctx.frames);
   copyAudioBuffer((*_preFX)[1], (*_postFX)[1], ctx.frames);

    float volume = (*_volume);
#if (DEFAULT_BUFFER_CHANNELS == 2) 
    mulAudioBufferToValue((*_postFX)[0], ctx.frames, volume);
    mulAudioBufferToValue((*_postFX)[1], ctx.frames, volume);
#else
    uint8_t channels = _preFX->channels();
    channels = _postFX->channels();
    for(uint8_t i=0; i<channels; ++i) {
        mulAudioBufferToValue((*_postFX)[i], ctx.frames, volume);
    }
#endif

    //process pan

    copyAudioBuffer((*_postFX)[0], (*_outputs)[0], ctx.frames);
    copyAudioBuffer((*_postFX)[1], (*_outputs)[1], ctx.frames);


    return ctx.frames;
}

void Track::prepareToPlay() {

}

void Track::prepareToRecord() {
    //???
    if(_recordTarget)
        _recordTarget->startRecord();
}

void Track::stopPlaying() {

}

//from Timeline::stopRecording
void Track::stopRecording() {
    if(_record) {
        _record = false;
        
        if(_recordTarget)
            _recordTarget->stopRecord();

        //reinit only here because if track record is turned off there is no need for reinit
        FlatEvents::FlatResponse reinit;
        reinit.type = FlatEvents::FlatResponse::Type::ReinitTrackRecord;
        reinit.reinitTrackRecord.track = this;
        RtEngine::addRtResponse(reinit);
    }
}

Status Track::setRecordArm(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp) {
    bool record = floatToBool(ev.recordArm.recordState);
    ev.recordArm.track->_record = record;
    ev.recordArm.track->_recordSource = ev.recordArm.recordSource;

    resp.type = FlatEvents::FlatResponse::Type::RecordArm;
    resp.status = Status::Ok;
    resp.recordArm.track = ev.recordArm.track;
    resp.recordArm.recordState = ev.recordArm.recordState;
    resp.recordArm.recordSource = ev.recordArm.recordSource;
    return Status::Ok;
}

Status Track::reinitRecord(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp) {
    (void)resp;
    if(ev.reinitTrackRecord.status == Status::Ok) {
        ev.reinitTrackRecord.track->_record = true;
    }
    return Status::NotOk; //don't send response
}

bool Track::checkFileContainerNeedResize() {
    return false;
}

void Track::resizeFileContainer() {

}

//latencyToCompensate comes from RecordArm or ReinitRecord events...
bool Track::prepareAudioRecord(FileWorker * fw, frame_t latencyToCompensate) {
    if(_recordTarget != nullptr) {
        if(!_recordTarget->release(fw)) {
            LOG_ERROR("Failed to release record target");
            return false;
        }
        delete _recordTarget;
        _recordTarget = nullptr;
    }
    bool ret = false;

    _recordTarget = new AudioRecord(this);
    ret = _recordTarget->prepare(fw, latencyToCompensate);
    _recordTarget->parent = this;
    return ret;
}
    
bool Track::prepareMidiRecord(FileWorker * fw) {
    LOG_WARN("Midi recording now available yet");
    return true;
}

bool Track::releaseRecordTarget(FileWorker * fw) {
    //from record arm event...
    if(_recordTarget) { //when midi target be added remove this if
        if(_recordTarget->used()) _recordTarget->finalize();
        else _recordTarget->release(fw);
    }

    return true;
}

bool Track::AudioRecord::prepare(FileWorker * fw, frame_t latencyToCompensate) {
    _bufferInUse = AudioBufferManager::acquireRecord();

    std::string generated = getDateTime();
    generated.append(generateRandomName(4));
    std::string path = SettingsManager::getTmpRecordPath();
    path.append(generated);
    path.append(".wav");

    LOG_INFO("Preparing audio file for record %s", path.c_str());
    _recordFile = fw->acquireTmpAudioFile();
    _recordFile->createTemporary(path, DEFAULT_BUFFER_CHANNELS, SettingsManager::getSampleRate());
    
    _fileUsed = false;
    _dumpOldBuffer = false;
    _oldBuffer = nullptr;
    _latencyToCompensate = latencyToCompensate;//driverInputLatenct + driverOutputLatency
    _compensatedLatency = 0;
    _samplesOffset = 0;
    return true;
}

bool Track::AudioRecord::release(FileWorker * fw) {
    // for(int i=0; i<RECORD_BUFFER_COUNT; ++i) {
    //     AudioBufferManager::releaseRecord(_recordBuffers[i].buf);
    //     //request to clear buffers in ce
    // }

    if(!_fileUsed) {
        fw->releaseTmpAudioFile(_recordFile);
    }

    return true;
}

void Track::AudioRecord::startRecord() {
    
}

//from rt Track::stopRecord
void Track::AudioRecord::stopRecord() {
    finalize();
}

void Track::AudioRecord::finalize() {
    _recordFile->finalizeFile();

    dumpDataCommand(_bufferInUse, _recordFile, _currentBufferFill);
    _currentBufferFill = 0;
}

void Track::AudioRecord::incrementCounter(frame_t frames) {
    _currentBufferFill += frames;
    
    if(_currentBufferFill >= _bufferInUse->bufferSize()) {
        _currentBufferFill = 0;
    }

    if(_oldBuffer) {
        _compensatedLatency += frames;
        if(_compensatedLatency >= _latencyToCompensate) {
            _dumpOldBuffer = true;
        }
    }

    if(_currentBufferFill == 0) {
        _oldBuffer = _bufferInUse;
        _bufferInUse = AudioBufferManager::acquireRecord();
    }

    if(_dumpOldBuffer) {
        dumpDataCommand(_oldBuffer, _recordFile, _oldBuffer->bufferSize());
        _compensatedLatency = 0;
        _dumpOldBuffer = false;
        _oldBuffer = nullptr;
    }
}
        
void Track::AudioRecord::writeData(void * data, frame_t frames, uint8_t numChannels, bool compensateLatency) {
    AudioBuffer * buffer = static_cast<AudioBuffer*>(data);
    frame_t readIdx = 0;
    frame_t writeIdx = 0;
    frame_t samplesToWrite = 0;

    if(!compensateLatency) {
        readIdx = 0;
        writeIdx = _currentBufferFill;
        samplesToWrite = frames;
    } else {
        long long idx = (long long)_currentBufferFill - (long long)_latencyToCompensate;
        long long test = idx+frames;
        
        if(idx < 0) {
            if(_oldBuffer) {
                readIdx = 0;
                writeIdx = _oldBuffer->bufferSize()+idx; //+ since idx is negative
                samplesToWrite = (test > 0) ? frames-test : frames;

                sumAudioBuffers(&(*buffer)[0][readIdx], &(*_oldBuffer)[0][writeIdx], samplesToWrite);
                sumAudioBuffers(&(*buffer)[1][readIdx], &(*_oldBuffer)[1][writeIdx], samplesToWrite);
            }
        }

        if(test < 0) return;

        long long tmp = frames - test;
        readIdx = (tmp > 0) ? tmp : 0;
        writeIdx = (tmp > 0) ? 0 : idx;
        samplesToWrite = (tmp > 0) ? test : frames;
    }

    if(samplesToWrite > 0) {
        sumAudioBuffers(&(*buffer)[0][readIdx], &(*_bufferInUse)[0][writeIdx], samplesToWrite);
        sumAudioBuffers(&(*buffer)[1][readIdx], &(*_bufferInUse)[1][writeIdx], samplesToWrite);
    }
}

void Track::AudioRecord::dumpDataCommand(AudioBuffer * buffer, AudioFile * file, frame_t size) {
    FlatEvents::FlatResponse dump;
    dump.type = FlatEvents::FlatResponse::Type::DumpRecordedAudio;
    dump.dumpRecordedAudio.targetBuffer = buffer;
    dump.dumpRecordedAudio.targetFile = file;
    dump.dumpRecordedAudio.size = size;
    dump.dumpRecordedAudio.trackId = _parent->id();
    RtEngine::addRtResponse(dump);

    _fileUsed = true;
}

        
}