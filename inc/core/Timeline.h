// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

// #include "core/FlatEvents.h"
// #include "common/Status.h"
#include "defines.h"

namespace slr {

class Project;

class Timeline {
    public:
    enum class RollState { Stop = 0, Pause, Preparing, Play };
    Timeline(Project & prj);
    ~Timeline();

    void init(int sampleRate, int blockSize);

    int timeToBars();
    int barsToTime();
    
    const bool playing() const { return (_state == RollState::Play) ? true : false; }
    const bool paused() const { return _state == RollState::Pause ? true : false; }
    const bool stopped() const { return _state == RollState::Stop ? true : false; }
    const bool recording() const { return _isRecording; }
    const bool looping() const { return _isLoop ? true : false; }
    const RollState state() const { return _state; }

    const frame_t elapsed(frame_t & framesPassed);
    frame_t lastElapsed() const { return _lastElapsed; }
    const frame_t loopStartFrame() const { return _loopStartFrame; }
    const frame_t loopEndFrame() const { return _loopEndFrame; }
    
    const float bpm() const { return _bpm; }

    const int ppqn() const { return _ppqn; }

    const BarSize getBarSize() const { return _size; }
    
    const int sampleRate() const { return _sampleRate; }
    const int blockSize() const { return _blockSize; }
    const uint32_t framesPerQuater() const { return _framesPerQuater; }

    [[deprecated]] //use stepToFrames in future
    const uint32_t framesPerBeat() const { return calcFramesPerBeat(); }
    const uint32_t framesPerBar() const { return calcFramesPerBar(); }
    const frame_t framesInStep(StepDuration dur) const;
    // const frame_t framesInStep(int dur) const;
    
    void setBpm(float newBpm);
    void setBarSize(BarSize size);
    void setTimelineState(TimelineState state);
    void setLoopPosition(frame_t start, frame_t end);
    void setLoopState(bool newState);

    private:
    float _bpm;
    BarSize _size;
    int _ppqn;
    int _sampleRate;
    int _blockSize;

    RollState _state;
    bool _isRecording;
    bool _isLoop;

    bool _resumed;
    frame_t _frameOnPaused;

    frame_t _startCountFrame;

    frame_t _lastLoopStamp;
    frame_t _loopStartFrame;
    frame_t _loopEndFrame;
    bool _loopOnGoing; //internal thing

    Project & _prj;

    uint32_t _framesPerQuater;
    void calcFramesPerQuater();
    uint32_t calcFramesPerBeat() const;
    uint32_t calcFramesPerBar() const;

    frame_t _lastElapsed; //for ui updates

    void play();
    void record();
    void stopRecord();
    void pause();
    void stop();
    void resumePlay(); //use after pause
    
};

}