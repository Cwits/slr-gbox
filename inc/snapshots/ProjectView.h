// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "snapshots/TimelineView.h"
#include "snapshots/FileContainerView.h"
#include "core/primitives/AudioRoute.h"
#include "core/primitives/MidiRoute.h"
#include "defines.h"
#include <vector>
#include <atomic>
#include <memory>

namespace slr {

class Timeline;
class ControlContext;
class Module;
class AudioUnit;
class AudioUnitView;

class ProjectView {
    public:
    ProjectView(Timeline * tl);
    ~ProjectView();

    AudioUnitView * createUnitView(const ControlContext &ctx, const Module *mod, AudioUnit * au);
    const std::size_t unitCount() const { return _unitViewList.size(); }
    std::vector<AudioUnitView*> unitList();
    AudioUnitView * getUnitById(ID id);
    AudioUnitView * removeUnitView(ID id);

    void updateRoutes(const std::vector<AudioRoute> & routes);
    const std::vector<AudioRoute> & audioRoutes() const { return _routes; }
    const std::vector<AudioRoute> sourcesForId(ID id);
    const std::vector<AudioRoute> targetsForId(ID id);
    
    void updateRoutes(const std::vector<MidiRoute> & routes);
    const std::vector<MidiRoute> & midiRoutes() const { return _midiRoutes; }
    const std::vector<MidiRoute> midiSourcesForId(ID id);
    const std::vector<MidiRoute> midiTargetsForId(ID id);

    TimelineView & timeline() { return _timeline; }
    static ProjectView & getProjectView();

    const uint64_t version() const { return _version.load(std::memory_order_acquire); }

    ClipItemView * createClipView(ClipItem *item);
    ClipItemView * findClipViewById(ID id);
    bool deleteClipViewById(ID id);

    private:
    std::vector<std::unique_ptr<AudioUnitView>> _unitViewList;

    TimelineView _timeline;
    std::vector<AudioRoute> _routes;
    std::vector<MidiRoute> _midiRoutes;

    ClipViewStorage _clipStorage;

    void incrementVersion() { _version.fetch_add(1, std::memory_order_release); }
    std::atomic<uint64_t> _version;
    // frame_t _playheadPosition;
};

}