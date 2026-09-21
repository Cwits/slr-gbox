// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/AudioUnit.h"
#include "core/primitives/RtTask.h"
#include "units/Track/TrackActions.h"
#include "common/defines.h"

namespace slr {

struct ParameterBase;
struct FileWorker;
struct BufferManager;
struct MidiFile;
struct RtEngine;
struct RecordTarget;

class Track : public AudioUnit {
    public:

    Track(const ClipContainer * initContainer, const ID forcedId);
    ~Track();
    
    bool create(BufferManager *man);
    bool destroy(BufferManager *man);

    frame_t process(const AudioContext &ctx,  const Dependencies &inputs) const override;

    void prepareToPlay() override;
    void prepareToRecord() override;
    void stopPlaying() override;
    void stopRecording() override;

    // const AudioBuffer * outputs() const { return _postFX; }

    const bool record() const { return _record; }
    inline void setRecord(bool state) { _record = state; }
    const RecordSource recordSource() const { return _recordSource; }
    inline void setRecordSource(RecordSource src) { _recordSource = src; }

    bool releaseRecordTarget();

    bool prepareRecord(std::unique_ptr<RecordTarget> target);
    
    private:
    BufferManager * _bufferManager;

    AudioBuffer * _recInt;
    AudioBuffer * _recExt;
    AudioBuffer * _preFX;
    AudioBuffer * _postFX;
    AudioBuffer * _postPan;

    MidiBuffer * _midiRecord;

    bool _record;

    //need to forbid to change source while recording == true
    RecordSource _recordSource = RecordSource::Audio;
    std::unique_ptr<RecordTarget> _recordTarget;

    RtTasks::ReinitTrackFlat _reinitFlat;
    RtTask _reinitTask;

    //monitor arm
    friend class RecordTarget;
    friend class AudioRecord;
    friend class MidiRecord;
};


struct RecordTarget {
    RecordTarget() {}
    virtual ~RecordTarget() {}
    // virtual bool release(FileWorker * fw) = 0;

    virtual void startRecord() = 0;
    virtual void stopRecord() = 0;
    virtual void incrementCounter(frame_t frames) = 0;
    virtual void finalize() = 0;
    virtual void writeData(void * data, frame_t frames, uint8_t numChannels, bool compensateLatency) = 0;
    virtual void clear() {
        _fileUsed = false;
        _finalizeRequested = false;
        _firstWrite = true;
        _fileStartPosition = 0;
        _parent = nullptr;
    }

    void setFileStartPosition(frame_t frame) { _fileStartPosition = frame; }
    bool isFirstWrite() const { return _firstWrite; }
    void markDirty() { _firstWrite = false; }
    const frame_t & fileStartPosition() { return _fileStartPosition; }

    bool used() const { return _fileUsed; }

    protected:
    bool _fileUsed = false;
    bool _finalizeRequested = false;
    bool _firstWrite = true;
    frame_t _fileStartPosition = 0;

    Track * _parent = nullptr;
    // void (*_dumpCommand)(RtTask * task);
    RtEngine * _engine;
    FileWorker * _fileWorker;

    friend class RecordArmAction;
    friend class Track;
};

struct AudioRecord : public RecordTarget {
    // explicit AudioRecord(Track * parent) : _parent(parent) {}
    AudioRecord() {}
    virtual ~AudioRecord();
    // bool prepare(FileWorker * fw, frame_t latencyToCompensate) override;
    // bool release(FileWorker * fw) override;
        
    void startRecord() override;
    void stopRecord() override;
    void incrementCounter(frame_t frames) override;
    void finalize() override;
    void writeData(void * data, frame_t frames, uint8_t numChannels, bool compensateLatency) override;
    void clear() override {
        RecordTarget::clear();
        _recordFile = nullptr;
        _latencyToCompensate = 0;
        _compensatedLatency = 0;
        _samplesOffset = 0;
        _dumpOldBuffer = false;
        _bufferInUse = nullptr;
        _oldBuffer = nullptr;
        _currentBufferFill = 0;
    }

    private:
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

    friend class RecordArmAction;
    friend class Track;
};

struct MidiRecord : public RecordTarget {
    // explicit MidiRecord(Track * parent) : _parent(parent) {}
    MidiRecord() {}
    virtual ~MidiRecord() = default;
    // virtual bool prepare(FileWorker * fw, frame_t latencyToCompensate) override;
    // virtual bool release(FileWorker * fw) override;

    virtual void startRecord() override;
    virtual void stopRecord() override;
    virtual void incrementCounter(frame_t frames) override;
    virtual void finalize() override;
    virtual void writeData(void * data, frame_t frames, uint8_t numChannels, bool compensateLatency) override;
    virtual void clear() override {

    }

    private:
    MidiFile * _recordFile = nullptr;

    MidiBufferRecord * _bufferInUse = nullptr; 
    MidiBufferRecord * _oldBuffer = nullptr; 
    bool _dumpOldBuffer = false;

    void dumpDataCommand(MidiBuffer *buffer, MidiFile *file, frame_t size, frame_t fileStartPosition);

    friend class RecordArmAction;
    friend class Track;
};

}