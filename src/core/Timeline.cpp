// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/Timeline.h"
#include "core/Project.h"
#include "core/SettingsManager.h"
#include "defines.h"

namespace slr {

Timeline::Timeline(Project & prj) : 
    _bpm(SettingsManager::getBpm()), 
    _size({
        static_cast<uint8_t>(SettingsManager::getNumerator()), 
        static_cast<uint8_t>(SettingsManager::getDenominator())}),
    _ppqn(SettingsManager::getPpqn()),
    _sampleRate(SettingsManager::getSampleRate()),
    _blockSize(SettingsManager::getBlockSize()),
    _state(RollState::Stop),
    _isRecording(false),
    _isLoop(false),
    _resumed(false),
    _frameOnPaused(0),
    _startCountFrame(0),
    _lastLoopStamp(0),
    _loopStartFrame(0),
    _loopEndFrame(0),
    _loopOnGoing(false),
    _prj(prj),
    _framesPerQuater(0),
    _lastElapsed(0)
{
    calcFramesPerQuater();
    _loopEndFrame = _framesPerQuater*16;
}

Timeline::~Timeline() {

}

void Timeline::init(int sampleRate, int blockSize) {
    _sampleRate = sampleRate;
    _blockSize = blockSize;
    calcFramesPerQuater();
}

int Timeline::timeToBars() {
    return 0;
}

int Timeline::barsToTime() {
    return 0;
}

void Timeline::play() {
    //prepare everything
    _prj.prepareForPlay();

    _startCountFrame = 0;
    _loopOnGoing = false;
    _lastLoopStamp = 0;

    _state = RollState::Preparing;
}

void Timeline::record() {
    if(!_isRecording) {
        if(!playing()) play();

        //prepare everything recording
        _prj.prepareForRecord();

        _isRecording = true;
    }
}

void Timeline::stopRecord() {
    _isRecording = false;
    //stop everything recording
    _prj.stopAfterRecord();
}

void Timeline::pause() {
    //???
    _state = RollState::Pause;
}

void Timeline::resumePlay() {
    //???
    _resumed = true;
    _state = RollState::Play;
}

void Timeline::stop() {
    if(_isRecording) {
        //stop everything recording
        stopRecord();
    }

    //stop everything playing
    _prj.stopAfterPlay();
    _startCountFrame = 0;
    _state = RollState::Stop;
}

const frame_t Timeline::elapsed(frame_t & framesPassed) {
    frame_t ret = 0;
    if(_state == RollState::Play) {
        if(_resumed) {
            _resumed = false;
            frame_t diff = _frameOnPaused - _startCountFrame;
            _startCountFrame = framesPassed - diff;
        }

        ret = framesPassed - _startCountFrame;
    
        if(_isLoop) {
            //...
            frame_t calc = ret - _lastLoopStamp;
            
            if(_loopOnGoing) calc += _loopStartFrame;

            if(calc > _loopStartFrame) {
                if(calc+_blockSize >= _loopEndFrame) {
                    _lastLoopStamp = ret+_blockSize;
                    _loopOnGoing = true;
                }
            }

            ret = calc;
        }
    }
    else if(_state == RollState::Preparing) {
        if(_startCountFrame == 0) {
            _startCountFrame = framesPassed+_blockSize;
        }
        _state = RollState::Play;
    } else if(_state == RollState::Pause) {
        _frameOnPaused = framesPassed;
        //maybe it should be framesPassed - _blockSize;?
    }

    _lastElapsed = ret;

    return ret;
}

void Timeline::setBpm(float newBpm) {
    _bpm = newBpm;
    calcFramesPerQuater();
}

void Timeline::setBarSize(BarSize size) { 
    _size = size; 
}

const frame_t Timeline::framesInStep(StepDuration dur) const {
    float div;
    switch(dur) {
        case(StepDuration::dWhole):         div = 4.0f;             break;
        case(StepDuration::dHalf):          div = 2.0f;             break;
        case(StepDuration::d4triplet):      div = 1.0f / 3.0f;      break;
        case(StepDuration::d4):             div = 1.0f;             break;
        case(StepDuration::d8tirplet):      div = 1.0f / 6.0f;      break;
        case(StepDuration::d8):             div = 0.5f;             break;
        case(StepDuration::d16triplet):     div = 1.0f / 12.0f;     break;
        case(StepDuration::d16):            div = 0.250f;           break;
        case(StepDuration::d32triplet):     div = 1.0f / 24.0f;     break;
        case(StepDuration::d32):            div = 0.125f;           break;
        case(StepDuration::d64triplet):     div = 1.0f / 48.0f;     break;
        case(StepDuration::d64):            div = 0.0625f;          break;
        case(StepDuration::d128triplet):    div = 1.0f / 96.0f;     break;
        case(StepDuration::d128):           div = 0.03125f;         break;
    }

    return framesPerQuater() * div;
}

void Timeline::calcFramesPerQuater() {
    _framesPerQuater = _sampleRate / (_bpm/60.f);
    //TODO: update loop markers as well
}

uint32_t Timeline::calcFramesPerBeat() const {
    float coef = 4.0f/getBarSize()._denominator;
    int framesPerQuaterNote = _framesPerQuater;
    return framesPerQuaterNote * coef;
}

uint32_t Timeline::calcFramesPerBar() const {
    //return framesInStep(getBarSize()._denominator) * getBarSize()._numerator;
    return framesPerBeat() * getBarSize()._numerator;
}

void Timeline::setTimelineState(TimelineState state) {
    switch(state) {
        case(TimelineState::Stop): stop(); break;
        case(TimelineState::Play): play(); break;
        case(TimelineState::Pause): pause(); break;
        case(TimelineState::StartRecord): record(); break;
        case(TimelineState::StopRecord): stopRecord(); break;
    }
}

void Timeline::setLoopPosition(frame_t start, frame_t end) {
    _loopStartFrame = start;
    _loopEndFrame = end;
}

void Timeline::setLoopState(bool newState) {
    _isLoop = newState;
    if(newState == true) {
        _lastLoopStamp = 0;
    } else {
        _startCountFrame += _lastLoopStamp;
    }
}

}