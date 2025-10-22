// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/Project.h"
#include "core/AudioBufferManager.h"
#include "core/primitives/AudioUnit.h"
#include "core/Metronome.h"
#include "logger.h"

#include <algorithm>

#define INITIAL_UNIT_SIZE 10

namespace slr {

Project::Project() : _timeline(*this) {
    _unitList.reserve(INITIAL_UNIT_SIZE);
    _soloPlan = nullptr;
    _renderPlan1 = nullptr;
    _renderPlan2 = nullptr;

    _metronome = std::make_unique<Metronome>();

    _renderPlan1 = buildPlan(this, routes());
    _renderPlan2 = buildPlan(this, routes());
    _soloPlan = buildPlan(this, routes());

    _isSolo = false;
    _planInWork = false;
}

Project::~Project() {
    destroyPlan(_soloPlan);
    destroyPlan(_renderPlan1);
    destroyPlan(_renderPlan2);
}

bool Project::addUnit(std::unique_ptr<AudioUnit> unit) {
    AudioUnit * u = unit.get();
    _unitList.push_back(std::move(unit));

    return true;
}

bool Project::removeUnit(ID id) {
    std::size_t size = _unitList.size();
    for(std::size_t i=0; i<size; ++i) {
        // if(_trackList.at(i) == nullptr) continue;
        if(_unitList.at(i).get()->id() == id) {
            _unitList.erase(_unitList.begin()+i);
            return true;
        }
    }
    return false;
}

AudioUnit * Project::getUnitById(ID id) {
    std::size_t size = _unitList.size();
    for(std::size_t i=0; i<size; ++i) {
        if(_unitList.at(i).get()->id() == id) return _unitList.at(i).get();
    }

    // for(int i=0; i<MAXIMUM_TRACKS; ++i) {
    //     if(_trackList[i] == nullptr) continue;
    //     if(_trackList[i].get()->getId() == id) return _trackList[i].get();
    // }

    LOG_ERROR("Wrong Unit ID");
    return nullptr;
}

Metronome * Project::metronome() const {
    return _metronome.get();
}

void Project::prepareForRecord() {
    for(auto & au : _unitList) {
        au->prepareToRecord();
    }
    _metronome->prepareToRecord();
}

void Project::prepareForPlay() {
    for(auto & au : _unitList) {
        au->prepareToPlay();
    }
    _metronome->prepareToPlay();
}

void Project::stopAfterRecord() {
    for(auto & au : _unitList) {
        au->stopRecording();
    }
    _metronome->stopRecording();
}

void Project::stopAfterPlay() {
    for(auto & au : _unitList) {
        au->stopPlaying();
    }
    _metronome->stopPlaying();
}

bool Project::evaluateRoute(const AudioRoute & route) {
    LOG_WARN("Routing evaluation not implemented yet");
    return true;
}

void Project::replaceEditablePlan(RenderPlan * plan) {
    if(editablePlan() == _renderPlan1) {
        destroyPlan(_renderPlan1);
        _renderPlan1 = plan;
    } else {
        destroyPlan(_renderPlan2);
        _renderPlan2 = plan;
    }
}

Status Project::swapPlan(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp) {
    Project * prj = ev.swapRenderPlan.project;
    
    prj->_planInWork ? prj->_planInWork = false : prj->_planInWork = true;
    
    resp.type = FlatEvents::FlatResponse::Type::SwapRenderPlan;
    resp.status = Status::Ok;
    resp.commandId = ev.commandId;
    return Status::Ok;
}

const RenderPlan * Project::editablePlan() const {
    return (_planInWork ? _renderPlan2 : _renderPlan1);
}

const RenderPlan * Project::runPlan() const {
    return (_planInWork ? _renderPlan1 : _renderPlan2);
}

const RenderPlan * Project::soloPlan() const {
    return _soloPlan;
}

void Project::removeRoutesForId(ID id) {
    _routes.erase(std::remove_if(_routes.begin(), _routes.end(), 
        [&id](const AudioRoute &r) {
            if(r._sourceId == id || r._targetId == id) {
                return true;
            } else {
                return false;
            }
    }), _routes.end());

}

}