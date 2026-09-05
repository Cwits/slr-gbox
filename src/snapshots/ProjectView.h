// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "snapshots/TimelineView.h"
#include "snapshots/FileContainerView.h"
#include "core/primitives/AudioRoute.h"
#include "core/primitives/MidiRoute.h"
#include "common/defines.h"
#include <vector>
#include <atomic>
#include <memory>
#include <string>

namespace slr {

class Timeline;
class ControlContext;
class UnitDescriptor;
class AudioUnit;
class AudioUnitView;
struct SequencerEngineView;
struct ModulationEngineView;

class ProjectView {
    public:
    ProjectView(Timeline * tl);
    ~ProjectView();

    std::shared_ptr<AudioUnitView> createUnitView(const ControlContext &ctx, const UnitDescriptor *desc, AudioUnit * au);
    const std::size_t unitCount() const { return _unitViewList.size(); }
    std::vector<AudioUnitView*> unitList();
    AudioUnitView * getUnitById(ID id);
    AudioUnitView * findUnitByName(const std::string &name);
    std::shared_ptr<AudioUnitView> removeUnitView(ID id);
    void appendUnit(std::shared_ptr<AudioUnitView> view);

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

    ClipItemView * createClipView(const ClipItem *item);
    ClipItemView * findClipViewById(ID id);
    bool deleteClipViewById(ID id);


    const std::string name() const { return _name; }
    void name(std::string n) { _name = n; }
    const std::string path() const { return _path; }
    void path(std::string p) { _path = p; }
    
    SequencerEngineView * stepSequencer() const { return _stepSequencer.get(); }
    ModulationEngineView * modulationEngine() const { return _modulationEngine.get(); }
    private:
    std::string _path;
    std::string _name;

    std::vector<std::shared_ptr<AudioUnitView>> _unitViewList;

    TimelineView _timeline;
    std::vector<AudioRoute> _routes;
    std::vector<MidiRoute> _midiRoutes;

    std::unique_ptr<SequencerEngineView> _stepSequencer;
    std::unique_ptr<ModulationEngineView> _modulationEngine;

    ClipViewStorage _clipStorage;

    void incrementVersion() { _version.fetch_add(1, std::memory_order_release); }
    std::atomic<uint64_t> _version;
    // frame_t _playheadPosition;
};

}