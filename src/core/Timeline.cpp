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
    return framesPerBeat() * getBarSize()._numerator;
}


/* RT Callbacks */
Status Timeline::setBpmTimeSig(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp) {
    ev.changeSigBpm.tl->setBpm(ev.changeSigBpm.bpm);
    ev.changeSigBpm.tl->setBarSize(ev.changeSigBpm.sig);

    resp.type = FlatEvents::FlatResponse::Type::ChangeSigBpm;
    resp.status = Status::Ok;
    resp.changeSigBpm.tl = ev.changeSigBpm.tl;
    resp.changeSigBpm.bpm = ev.changeSigBpm.bpm;
    resp.changeSigBpm.sig = ev.changeSigBpm.sig;
    return Status::Ok;
}

Status Timeline::setLoopPosition(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp) {
    ev.loopPosition.tl->setLoopStartFrame(ev.loopPosition.start);
    ev.loopPosition.tl->setLoopEndFrame(ev.loopPosition.end);

    resp.type = FlatEvents::FlatResponse::Type::LoopPosition;
    resp.status = Status::Ok;
    resp.loopPosition.tl = ev.loopPosition.tl;
    resp.loopPosition.start = ev.loopPosition.start;
    resp.loopPosition.end = ev.loopPosition.end;
    return Status::Ok;
}

Status Timeline::toggleLoop(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp) {
    ev.toggleLoop.timeline->setLoop(ev.toggleLoop.newState);
    if(ev.toggleLoop.newState == true) {
        ev.toggleLoop.timeline->_lastLoopStamp = 0;
    } else {
        ev.toggleLoop.timeline->_startCountFrame += ev.toggleLoop.timeline->_lastLoopStamp;
    }

    resp.type = FlatEvents::FlatResponse::Type::ToggleLoop;
    resp.status = Status::Ok;
    resp.toggleLoop.timeline = ev.toggleLoop.timeline;
    resp.toggleLoop.newState = ev.toggleLoop.newState;
    return Status::Ok;
}

Status Timeline::changeTimelineState(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp) {
    resp.type = FlatEvents::FlatResponse::Type::ChangeTimelineState;
    switch(ev.changeTimelineState.state) {
        case(TimelineState::Stop): ev.changeTimelineState.timeline->stop(); break;
        case(TimelineState::Play): ev.changeTimelineState.timeline->play(); break;
        case(TimelineState::Pause): ev.changeTimelineState.timeline->pause(); break;
        case(TimelineState::StartRecord): ev.changeTimelineState.timeline->record(); break;
        case(TimelineState::StopRecord): ev.changeTimelineState.timeline->stopRecord(); break;
    }
    resp.changeTimelineState.state = ev.changeTimelineState.state;
    resp.status = Status::Ok;
    return resp.status;
}

Status Timeline::requestPlayhead(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp) {
    resp.type = FlatEvents::FlatResponse::Type::RequestPlayhead;
    resp.requestPlayhead.position = ev.requestPlayhead.timeline->_lastElapsed;
    resp.status = Status::Ok;
    return Status::Ok;
}

}