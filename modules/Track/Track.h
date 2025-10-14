// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/AudioUnit.h"
#include "core/primitives/AudioBuffer.h"
#include "core/FlatEvents.h"
#include "defines.h"
#include "Status.h"

namespace slr {

class ParameterBase;
class FileWorker;

struct RecordBuffer { 
    enum class State { Idle, Filling, Full, Processing };
    AudioBuffer * buf;
    State state;
};

class Track : public AudioUnit {
    public:

    Track();
    ~Track();
    RT_FUNC frame_t process(const AudioContext &ctx,  const Dependencies * const inputs, const uint32_t inputsCount) override;

    RT_FUNC void prepareToPlay() override;
    RT_FUNC void prepareToRecord() override;
    RT_FUNC void stopPlaying() override;
    RT_FUNC void stopRecording() override;

    // const AudioBuffer * outputs() const { return _postFX; }

    RT_FUNC static Status setRecordArm(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp);
    RT_FUNC static Status reinitRecord(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp);

    const bool record() const { return _record; }
    const RecordSource recordSource() const { return _recordSource; }
    
    /* NON RT Use only!!! */
    
    bool checkFileContainerNeedResize();
    void resizeFileContainer();
    
    bool prepareAudioRecord(FileWorker * fw, frame_t latencyToCompensate);
    bool prepareMidiRecord(FileWorker * fw);
    bool releaseRecordTarget(FileWorker * fw);
    
    private:
    AudioBuffer * _recInt;
    AudioBuffer * _recExt;
    AudioBuffer * _preFX;
    AudioBuffer * _postFX;
    AudioBuffer * _postPan;
    
    bool _buffersClear;

    bool _record;

    static constexpr int RECORD_BUFFER_COUNT = 4;
    struct RecordTarget {
        virtual ~RecordTarget() {}
        virtual bool prepare(FileWorker * fw, frame_t latencyToCompensate) = 0;
        virtual bool release(FileWorker * fw) = 0;

        RT_FUNC virtual void startRecord() = 0;
        RT_FUNC virtual void stopRecord() = 0;
        RT_FUNC virtual void incrementCounter(frame_t frames) = 0;
        RT_FUNC virtual void finalize() = 0;
        RT_FUNC virtual void writeData(void * data, frame_t frames, uint8_t numChannels, bool compensateLatency) = 0;
        
        bool used() const { return _fileUsed; }
        Track * parent = nullptr;
        protected:
        bool _fileUsed = false;
        bool _finalizeRequested = false;
    };

    struct AudioRecord : public RecordTarget {
        explicit AudioRecord(Track * parent) : _parent(parent) {}
        virtual ~AudioRecord() = default;
        bool prepare(FileWorker * fw, frame_t latencyToCompensate) override;
        bool release(FileWorker * fw) override;
        
        RT_FUNC void startRecord() override;
        RT_FUNC void stopRecord() override;
        RT_FUNC void incrementCounter(frame_t frames) override;
        RT_FUNC void finalize() override;
        RT_FUNC void writeData(void * data, frame_t frames, uint8_t numChannels, bool compensateLatency) override;
        private:
        Track * _parent = nullptr;
        AudioFile * _recordFile = nullptr;

        frame_t _latencyToCompensate = 0;
        frame_t _compensatedLatency = 0;
        frame_t _samplesOffset = 0;

        bool _dumpOldBuffer = false;
        AudioBuffer * _bufferInUse = nullptr;
        AudioBuffer * _oldBuffer = nullptr;
        frame_t _currentBufferFill = 0;

        void dumpDataCommand(AudioBuffer * buffer, AudioFile * file, frame_t size/*, const AudioContext &ctx*/);
    };

    // struct MidiRecord : public RecordTarget {
    //     void prepare() override;
    //     void release() override;
    //     void startReocord() override;
    //     void stopRecord() override;
    //     void writeData(void * data, frame_t frames) override;
    //     //midi file
    // };

    //need to forbid to change source while recording == true
    RecordSource _recordSource = RecordSource::Audio;
    RecordTarget * _recordTarget;

    //midibuffer midirecordbuffer;
    //fx chain
    //monitor arm
    //selected input (type and source)
};

}