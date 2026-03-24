// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"

#include "core/Timeline.h"
#include "core/ControlEngine.h"
#include "core/primitives/ControlContext.h"
#include "core/ModuleManager.h"

#include "logger.h"

namespace slr {


ProjectView::ProjectView(Timeline *tl) : _timeline(tl) {
    // _playheadPosition = 0;
}

ProjectView::~ProjectView() {
}

std::vector<AudioUnitView*> ProjectView::unitList() {
    std::vector<AudioUnitView*> ret;
    for(std::unique_ptr<AudioUnitView>& v : _unitViewList) {
        ret.push_back(v.get());
    }

    return ret;
}

AudioUnitView * ProjectView::createUnitView(const ControlContext &ctx, const Module *mod, AudioUnit * au) {
    AudioUnitView * view = nullptr;
    try {
        std::unique_ptr<AudioUnitView> v = mod->createView(au);
        view = v.get();
        _unitViewList.push_back(std::move(v));
        incrementVersion();
    } catch(...) {
        LOG_ERROR("Failed to create %s", mod->_name->data());
    }
    return view;
}

AudioUnitView * ProjectView::getUnitById(ID id) {
    AudioUnitView * unit = nullptr;
    for(std::size_t i=0; i<_unitViewList.size(); ++i) {
        AudioUnitView * potential = _unitViewList.at(i).get();
        if(potential->id() == id) {
            unit = potential;
        }
    }

    return unit;
}

AudioUnitView * ProjectView::removeUnitView(ID id) {
    std::size_t pos = 0;
    bool found = false;
    for(std::size_t i=0; i<_unitViewList.size(); ++i) {
        AudioUnitView * potential = _unitViewList.at(i).get();
        if(potential->id() == id) {
            pos = i;
            found = true;
        }
    }

    AudioUnitView * unit = nullptr;
    if(found) {
        unit = _unitViewList.at(pos).get();
        _unitViewList.erase(_unitViewList.begin() + pos);
        incrementVersion();
    } else {
        LOG_ERROR("Failed to find unit with id %u", id);
    }
    return unit;
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

ClipItemView * ProjectView::createClipView(ClipItem *item) {
    return _clipStorage.newClipView(item);
}

ClipItemView * ProjectView::findClipViewById(ID id) {
    return _clipStorage.findClipById(id);
}

bool ProjectView::deleteClipViewById(ID id) {
    return _clipStorage.deleteClipById(id);
}

}