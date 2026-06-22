// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "defines.h"
#include "core/primitives/MidiEvent.h"
#include "core/StepSequencer.h"
#include "common/Color.h"

#include <vector>
#include <memory>

namespace slr {

struct LayerView {
    ID _targetId;

    std::array<bool, EVENTS_COUNT> _events;
    uint8_t _note;
    uint8_t _velocity;

    bool _mute;
    bool _active;
};

struct SequenceView {
    SequenceView(const Sequence* seq);
    ~SequenceView();

    std::array<ID, TARGET_COUNT> targetId() const { return _targets; }

    const ID id() const;
    int activeLayerCount() const;
    const LayerView& layer(unsigned int id) const;
    const std::array<LayerView, LAYERS_COUNT> & layers() { return _layers; }

    MidiEvent getEvent(unsigned int layer, unsigned int event) const;

    StepDuration stepDuration() const { return _stepDuration; }
    uint32_t stepCount() const { return _stepCount; }
    
    void update(bool sizeChanged); //if true -> need to check for _steps
    
    const Color & color() const { return _uniqueColor; }
    void color(Color clr) { _uniqueColor = clr; incrementVersion(); }

    uint64_t version() const { return _version.load(std::memory_order_acquire); }
    
    private:
    const Sequence *_seq;

    std::array<ID, TARGET_COUNT> _targets;
    uint32_t _stepCount;

    std::array<LayerView, LAYERS_COUNT> _layers;
    StepDuration _stepDuration;

    //add uniqueColor;
    Color _uniqueColor;

    void incrementVersion() { _version.fetch_add(1, std::memory_order_release); }
    std::atomic<uint64_t> _version;
};

struct SequencerEngineView {
    SequencerEngineView();
    ~SequencerEngineView();

    // std::shared_ptr<SequenceView> createSequenceView(const Sequence *seq);
    std::shared_ptr<SequenceView> createSequenceView(const Sequence *seq);
    std::shared_ptr<SequenceView> findSequenceById(ID id);

    private:
    // std::vector<std::shared_ptr<SequenceView>> _sequences;
    std::vector<std::shared_ptr<SequenceView>> _sequences;
};

}