// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/AudioUnit.h"
#include "core/primitives/RtTask.h"
#include "modules/Track/TrackActions.h"
#include "defines.h"

namespace slr {

class ParameterBase;
class FileWorker;
class BufferManager;
class MidiFile;

class Track : public AudioUnit {
    public:

    Track(const ClipContainer * initContainer);
    ~Track();
    
    bool create(BufferManager *man);
    bool destroy(BufferManager *man);

    RT_FUNC frame_t process(const AudioContext &ctx,  const Dependencies &inputs) override;

    RT_FUNC void prepareToPlay() override;
    RT_FUNC void prepareToRecord() override;
    RT_FUNC void stopPlaying() override;
    RT_FUNC void stopRecording() override;

    // const AudioBuffer * outputs() const { return _postFX; }

    const bool record() const { return _record; }
    inline void setRecord(bool state) { _record = state; }
    const RecordSource recordSource() const { return _recordSource; }
    inline void setRecordSource(RecordSource src) { _recordSource = src; }

    bool prepareAudioRecord(FileWorker * fw, frame_t latencyToCompensate);
    bool prepareMidiRecord(FileWorker * fw);
    bool releaseRecordTarget(FileWorker * fw);

    private:
    BufferManager * _bufferManager;

    AudioBuffer * _recInt;
    AudioBuffer * _recExt;
    AudioBuffer * _preFX;
    AudioBuffer * _postFX;
    AudioBuffer * _postPan;

    MidiBuffer * _midiRecord;

    bool _record;

    struct RecordTarget {
        virtual ~RecordTarget() {}
        virtual bool prepare(FileWorker * fw, frame_t latencyToCompensate) = 0;
        virtual bool release(FileWorker * fw) = 0;

        RT_FUNC virtual void startRecord() = 0;
        RT_FUNC virtual void stopRecord() = 0;
        RT_FUNC virtual void incrementCounter(frame_t frames) = 0;
        RT_FUNC virtual void finalize() = 0;
        RT_FUNC virtual void writeData(void * data, frame_t frames, uint8_t numChannels, bool compensateLatency) = 0;
        
        RT_FUNC void setFileStartPosition(frame_t frame) { _fileStartPosition = frame; }
        RT_FUNC bool isFirstWrite() const { return _firstWrite; }
        RT_FUNC void markDirty() { _firstWrite = false; }
        RT_FUNC const frame_t & fileStartPosition() { return _fileStartPosition; }

        bool used() const { return _fileUsed; }
        // Track * parent = nullptr;
        protected:
        bool _fileUsed = false;
        bool _finalizeRequested = false;
        bool _firstWrite = true;
        frame_t _fileStartPosition = 0;
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

        void dumpDataCommand(AudioBuffer * buffer, AudioFile * file, frame_t size, frame_t fileStartPosition/*, const AudioContext &ctx*/);
    
        
        RtTasks::DumpAudioFlat _flat;
        RtTask _task;
    };

    struct MidiRecord : public RecordTarget {
        explicit MidiRecord(Track * parent) : _parent(parent) {}
        virtual ~MidiRecord() = default;
        virtual bool prepare(FileWorker * fw, frame_t latencyToCompensate) override;
        virtual bool release(FileWorker * fw) override;

        RT_FUNC virtual void startRecord() override;
        RT_FUNC virtual void stopRecord() override;
        RT_FUNC virtual void incrementCounter(frame_t frames) override;
        RT_FUNC virtual void finalize() override;
        RT_FUNC virtual void writeData(void * data, frame_t frames, uint8_t numChannels, bool compensateLatency) override;
        
        private:
        Track * _parent = nullptr;
        MidiFile * _recordFile = nullptr;

        MidiBufferRecord * _bufferInUse = nullptr; 
        MidiBufferRecord * _oldBuffer = nullptr; 
        bool _dumpOldBuffer = false;

        void dumpDataCommand(MidiBuffer *buffer, MidiFile *file, frame_t size, frame_t fileStartPosition);
    };

    //need to forbid to change source while recording == true
    RecordSource _recordSource = RecordSource::Audio;
    RecordTarget * _recordTarget;

    RtTasks::ReinitTrackFlat _reinitFlat;
    RtTask _reinitTask;

    //monitor arm
};

}