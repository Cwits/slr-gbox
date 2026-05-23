// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"

#include "core/Timeline.h"
#include "core/ControlEngine.h"
#include "core/primitives/ControlContext.h"
#include "core/UnitManager.h"
#include "core/SettingsManager.h"

#include "logger.h"

namespace slr {


ProjectView::ProjectView(Timeline *tl) : _timeline(tl) {
    // _playheadPosition = 0;
    _name = "Untitled Project";
}

ProjectView::~ProjectView() {
}

std::vector<AudioUnitView*> ProjectView::unitList() {
    std::vector<AudioUnitView*> ret;
    for(std::shared_ptr<AudioUnitView>& v : _unitViewList) {
        ret.push_back(v.get());
    }

    return ret;
}

std::shared_ptr<AudioUnitView> ProjectView::createUnitView(const ControlContext &ctx, const UnitDescriptor *desc, AudioUnit * au) {
    std::shared_ptr<AudioUnitView> view;
    try {
        std::shared_ptr<AudioUnitView> v = desc->createView(au);
        view = v;
        _unitViewList.push_back(v);
        incrementVersion();
    } catch(...) {
        LOG_ERROR("Failed to create %s", desc->_name->data());
    }
    return view;
}

AudioUnitView * ProjectView::getUnitById(ID id) {
    AudioUnitView * unit = nullptr;
    if(id == 0) return unit; //id == 0 is metronome
    for(std::size_t i=0; i<_unitViewList.size(); ++i) {
        AudioUnitView * potential = _unitViewList.at(i).get();
        if(potential->id() == id) {
            unit = potential;
        }
    }

    return unit;
}

std::shared_ptr<AudioUnitView> ProjectView::removeUnitView(ID id) {
    std::size_t pos = 0;
    bool found = false;
    for(std::size_t i=0; i<_unitViewList.size(); ++i) {
        AudioUnitView * potential = _unitViewList.at(i).get();
        if(potential->id() == id) {
            pos = i;
            found = true;
        }
    }

    // AudioUnitView * unit = nullptr;
    if(found) {
        std::shared_ptr<AudioUnitView> unit = _unitViewList.at(pos);
        _unitViewList.erase(_unitViewList.begin() + pos);
        incrementVersion();
        return unit;
    } else {
        LOG_ERROR("Failed to find unit with id %u", id);
        return std::shared_ptr<AudioUnitView>();
    }
}

ProjectView & ProjectView::getProjectView() {
    return *ControlEngine::projectSnapshot();
}

void ProjectView::updateRoutes(const std::vector<AudioRoute> & routes) {
    _routes.clear();
    _routes = routes;
}

//routes where this id is source(sending to this id)
const std::vector<AudioRoute> ProjectView::sourcesForId(ID id) {
    std::vector<AudioRoute> ret;
    for(AudioRoute &r : _routes) {
        if(r._targetId == id) {
            ret.push_back(r);
        } 
    }

    return ret;
}

//routes where this id is target(sending from this id)
const std::vector<AudioRoute> ProjectView::targetsForId(ID id) {
    std::vector<AudioRoute> ret;
    for(AudioRoute &r : _routes) {
        if(r._sourceId == id) {
            ret.push_back(r);
        }
    }

    return ret;
}

void ProjectView::updateRoutes(const std::vector<MidiRoute> & routes) {
    _midiRoutes.clear();
    _midiRoutes = routes;
}

/*
//TODO: this logic is wrong... sources for Id must mean where current ID is target. and vice versa
same goes for audio routes
*/
const std::vector<MidiRoute> ProjectView::midiSourcesForId(ID id) {
    std::vector<MidiRoute> ret;
    for(MidiRoute &r : _midiRoutes) {
        if(r._sourceId == id) ret.push_back(r);
    }

    return ret;
}

const std::vector<MidiRoute> ProjectView::midiTargetsForId(ID id) {
    std::vector<MidiRoute> ret;
    for(MidiRoute &r : _midiRoutes) {
        if(r._targetId == id) ret.push_back(r);
    }
    
    return ret;
}

ClipItemView * ProjectView::createClipView(const ClipItem *item) {
    return _clipStorage.newClipView(item);
}

ClipItemView * ProjectView::findClipViewById(ID id) {
    return _clipStorage.findClipById(id);
}

bool ProjectView::deleteClipViewById(ID id) {
    return _clipStorage.deleteClipById(id);
}

}