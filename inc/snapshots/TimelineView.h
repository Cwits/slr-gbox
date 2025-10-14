// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"
#include "core/Timeline.h"

namespace slr {

class TimelineView {
    public:
    TimelineView(const Timeline * tl);
    ~TimelineView();

    BarSize getBarSize() const { return _size; }
    void setBarSize(BarSize size) { _size = size; }
    
    bool playing() const { return (_state == Timeline::RollState::Play) ? true : false; }
    bool recording() const { return _isRecording; }
    bool paused() const { return _state == Timeline::RollState::Pause ? true : false; }
    bool stopped() const { return _state == Timeline::RollState::Stop ? true : false; }
    bool looping() const { return _isLoop; }
    const Timeline::RollState state() const { return _state; }
    //TODO: add some extra logic here(e.g. playhead == 0 on state == Stop)
    void setState(Timeline::RollState newState) { _state = newState; }
    void setRecording(bool state) { _isRecording = state; }


    frame_t elapsed() { return _playhead; }
    void setPlayhead(frame_t pos) { _playhead = pos; }
    const frame_t loopStartFrame() const { return _loopStartFrame; }
    const frame_t loopEndFrame() const { return _loopEndFrame; }
    void setLoopStart(frame_t start) { _loopStartFrame = start; }
    void setLoopEnd(frame_t end) { _loopEndFrame = end; }


    const float bpm() const { return _bpm; }
    const int ppqn() const { return _ppqn; }
    
    const int sampleRate() const { return _sampleRate; }
    const int blockSize() const { return _blockSize; }
    const uint32_t framesPerQuater() const { return _framesPerQuater; }
    const uint32_t framesPerBeat() const { return _framesPerBeat; }
    const uint32_t framesPerBar() const { return _framesPerBar; }

    void update();
    void clone(TimelineView & other);

    static TimelineView & getTimelineView();

    private:
    float _bpm;
    BarSize _size;
    Timeline::RollState _state;
    bool _isRecording;
    bool _isLoop;
    int _ppqn;
    frame_t _playhead;
    frame_t _loopStartFrame;
    frame_t _loopEndFrame;
    int _sampleRate;
    int _blockSize;
    uint32_t _framesPerQuater;
    uint32_t _framesPerBeat;
    uint32_t _framesPerBar;

    const Timeline * _timeline;
};

}