// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "snapshots/TimelineView.h"
#include "core/Timeline.h"
#include "core/ControlEngine.h"
#include "snapshots/ProjectView.h"

namespace slr {


TimelineView::TimelineView(const Timeline * tl) :
    _timeline(tl),
    _bpm(tl->bpm()),
    _size(tl->getBarSize()),
    _state(tl->state()),
    _isRecording(tl->recording()),
    _isLoop(tl->looping()),
    _ppqn(tl->ppqn()),
    _playhead(0),
    _loopStartFrame(tl->loopStartFrame()),
    _loopEndFrame(tl->loopEndFrame()),
    _sampleRate(tl->sampleRate()),
    _blockSize(tl->blockSize()),
    _framesPerQuater(tl->framesPerQuater()),
    _framesPerBeat(tl->framesPerBeat()),
    _framesPerBar(tl->framesPerBar())
{

}

TimelineView::~TimelineView() {

}

/* does not update _playhead position */
void TimelineView::update() {
    _bpm = _timeline->bpm();
    _size = _timeline->getBarSize();
    _state = _timeline->state();
    _isRecording = _timeline->recording();
    _isLoop = _timeline->looping();
    _ppqn = _timeline->ppqn();
    _loopStartFrame = _timeline->loopStartFrame();
    _loopEndFrame = _timeline->loopEndFrame();
    _sampleRate = _timeline->sampleRate();
    _blockSize = _timeline->blockSize();
    _framesPerQuater = _timeline->framesPerQuater();
    _framesPerBeat = _timeline->framesPerBeat();
    _framesPerBar = _timeline->framesPerBar();
}

void TimelineView::clone(TimelineView & other) {
    _bpm = other.bpm();
    _size = other.getBarSize();
    _state = other.state();
    _isRecording = other.recording();
    _isLoop = other.looping();
    _ppqn = other.ppqn();
    _playhead = other.elapsed();
    _loopStartFrame = other.loopStartFrame();
    _loopEndFrame = other.loopEndFrame();
    _sampleRate = other.sampleRate();
    _blockSize = other.blockSize();
    _framesPerQuater = other.framesPerQuater();
    _framesPerBeat = other.framesPerBeat();
    _framesPerBar = other.framesPerBar();
}

TimelineView & TimelineView::getTimelineView() {
    return ControlEngine::projectSnapshot()->timeline();
}

}