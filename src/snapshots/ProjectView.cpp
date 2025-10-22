// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"

#include "core/Timeline.h"
#include "core/ControlEngine.h"

#include "logger.h"

namespace slr {


ProjectView::ProjectView(Timeline *tl) : _timeline(tl) {
    // _playheadPosition = 0;
}

ProjectView::~ProjectView() {
    //both Views points to same data...
    for(AudioUnitView * tr : _unitList) {
        delete tr;
    }
}

std::vector<AudioUnitView*> ProjectView::unitList() {
    std::vector<AudioUnitView*> ret;
    for(AudioUnitView * v : _unitList) {
        ret.push_back(v);
    }

    return ret;
}

AudioUnitView * ProjectView::getUnitById(ID id) {
    AudioUnitView * unit = nullptr;
    for(std::size_t i=0; i<_unitList.size(); ++i) {
        AudioUnitView * potential = _unitList.at(i);
        if(potential->id() == id) {
            unit = potential;
        }
    }

    return unit;
}

AudioUnitView * ProjectView::removeUnitView(ID id) {
    std::size_t pos = 0;
    bool found = false;
    for(std::size_t i=0; i<_unitList.size(); ++i) {
        AudioUnitView * potential = _unitList.at(i);
        if(potential->id() == id) {
            pos = i;
            found = true;
        }
    }

    AudioUnitView * unit = nullptr;
    if(found) {
        unit = _unitList.at(pos);
        _unitList.erase(_unitList.begin() + pos);
    } else {
        LOG_ERROR("Failed to find unit with id %u", id);
    }
    return unit;
}

void ProjectView::clone(ProjectView * other) {
    // std::size_t sizeThis = _trackList.size();
    // std::size_t sizeOther = other->_trackList.size();
    _timeline.clone(other->timeline());
    _unitList = other->_unitList;
    // _playheadPosition = other->_playheadPosition;
}

ProjectView & ProjectView::getProjectView() {
    return *ControlEngine::projectSnapshot();
}

void ProjectView::updateRoutes(const std::vector<AudioRoute> & routes) {
    _routes.clear();
    _routes = routes;
}

//routes where this id is source(receiving from this id)
const std::vector<AudioRoute> ProjectView::sourcesForId(ID id) {
    std::vector<AudioRoute> ret;
    for(AudioRoute &r : _routes) {
        if(r._sourceId == id) {
            ret.push_back(r);
        }
    }

    return ret;
}

//routes where this id is target(sending to this id)
const std::vector<AudioRoute> ProjectView::targetsForId(ID id) {
    std::vector<AudioRoute> ret;
    for(AudioRoute &r : _routes) {
        if(r._targetId == id) {
            ret.push_back(r);
        }
    }

    return ret;
}


}