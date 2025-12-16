// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "snapshots/TimelineView.h"
#include "core/primitives/AudioRoute.h"
#include "core/primitives/MidiRoute.h"
#include "defines.h"
#include <vector>

namespace slr {

class Timeline;
class AudioUnitView;

class ProjectView {
    public:
    ProjectView(Timeline * tl);
    ~ProjectView();
    void clone(ProjectView * other);

    void addUnitView(AudioUnitView * unit) { _unitList.push_back(unit); }
    const std::size_t unitCount() const { return _unitList.size(); }
    std::vector<AudioUnitView*> unitList();
    AudioUnitView * getUnitById(ID id);
    AudioUnitView * removeUnitView(ID id);

    void updateRoutes(const std::vector<AudioRoute> & routes);
    const std::vector<AudioRoute> sourcesForId(ID id);
    const std::vector<AudioRoute> targetsForId(ID id);
    
    void updateRoutes(const std::vector<MidiRoute> & routes);
    const std::vector<MidiRoute> midiSourcesForId(ID id);
    const std::vector<MidiRoute> midiTargetsForId(ID id);

    TimelineView & timeline() { return _timeline; }
    static ProjectView & getProjectView();

    private:
    std::vector<AudioUnitView*> _unitList;
    TimelineView _timeline;
    std::vector<AudioRoute> _routes;
    std::vector<MidiRoute> _midiRoutes;
    // frame_t _playheadPosition;
};

}