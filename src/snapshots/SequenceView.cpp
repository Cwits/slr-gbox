// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "snapshots/SequenceView.h"

#include "core/StepSequencer.h"
#include "core/primitives/AudioUnit.h"

#include "common/logger.h"
#include <algorithm>

namespace slr {

SequenceView::SequenceView(const Sequence *seq) :
    _seq(seq)
{
    for(int i=0; i<TARGET_COUNT; ++i) _targets[i] = _seq->_targets[i] ? _seq->_targets[i]->id() : 0;
    _stepCount = _seq->_stepCount;
    _stepDuration = _seq->stepDuration();

    for(int i=0; i<LAYERS_COUNT; ++i) {
        LayerView &lv = _layers[i];
        const Sequence::Layer &l = _seq->_layers[i];

        lv._targetId = (l._target ? l._target->id() : 0);
        lv._active = l._active;
        lv._mute = l._mute;
        lv._note = l._note;
        lv._velocity = l._velocity;
        for(int y=0; y<EVENTS_COUNT; ++y) {
            lv._events[y] = l._events[y]._enabled;
        }
    }

    _uniqueColor.r = rand()%255;
    _uniqueColor.g = rand()%255;
    _uniqueColor.b = rand()%255;
    _uniqueColor.a = 255;
}

SequenceView::~SequenceView() {

}

const ID SequenceView::id() const {
    return _seq->_uniqueId;
}

int SequenceView::activeLayerCount() const {
    int ret = 0;
    for(int i=0; i<LAYERS_COUNT; ++i) {
        if(_layers[i]._active) ret++;
    }

    return ret;
}

const LayerView & SequenceView::layer(unsigned int id) const {
    if(id >= LAYERS_COUNT) return LayerView{};
    else return _layers[id];
}

void SequenceView::update(bool sizeChanged) {
    for(int i=0; i<TARGET_COUNT; ++i) _targets[i] = _seq->_targets[i] ? _seq->_targets[i]->id() : 0;
    _stepCount = _seq->_stepCount;
    _stepDuration = _seq->stepDuration();

    for(int i=0; i<LAYERS_COUNT; ++i) {
        LayerView &lv = _layers[i];
        const Sequence::Layer &l = _seq->_layers[i];

        lv._targetId = (l._target ? l._target->id() : 0);
        lv._active = l._active;
        lv._mute = l._mute;
        lv._note = l._note;
        lv._velocity = l._velocity;
        lv._active = l._active;
        for(int y=0; y<EVENTS_COUNT; ++y) {
            lv._events[y] = l._events[y]._enabled;
        }
    }

    incrementVersion();
}

MidiEvent SequenceView::getEvent(unsigned int layer, unsigned int event) const {
    return _seq->_layers[layer]._events[event]._event;
}


SequencerEngineView::SequencerEngineView() {

}

SequencerEngineView::~SequencerEngineView() {

}

std::shared_ptr<SequenceView> SequencerEngineView::createSequenceView(const Sequence *seq) {
    _sequences.push_back(std::make_shared<SequenceView>(seq));

    return _sequences.back();
}

std::shared_ptr<SequenceView> SequencerEngineView::findSequenceById(ID id) {
    auto it = std::find_if(
        _sequences.begin(),
        _sequences.end(),
        [id](const auto &s) {
            return s->id() == id;
        }
    );

    if(it == _sequences.end()) {
        LOG_ERROR("Failed to find sequence id %lu", id);
        return nullptr;
    }

    return (*it);
}

}