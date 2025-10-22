// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <memory>
#include <array>

#include "core/primitives/RenderPlan.h"
#include "core/primitives/AudioRoute.h"
#include "core/Timeline.h"
#include "defines.h"

// #define MAXIMUM_TRACKS 64

namespace slr {

class AudioUnit;
class Metronome;

class Project {
    public:
    Project();
    ~Project();

    RT_FUNC void prepareForRecord();
    RT_FUNC void prepareForPlay();
    RT_FUNC void stopAfterRecord();
    RT_FUNC void stopAfterPlay();

    RT_FUNC const bool isSolo() const { return _isSolo; }

    RT_FUNC static Status swapPlan(const FlatEvents::FlatControl &ev, FlatEvents::FlatResponse &resp);

    const RenderPlan * editablePlan() const;
    void replaceEditablePlan(RenderPlan * plan);
    RT_FUNC const RenderPlan * runPlan() const;
    RT_FUNC const RenderPlan * soloPlan() const;

    //nonRT
    bool addUnit(std::unique_ptr<AudioUnit> unit);
    bool removeUnit(ID id);

    const int getUnitCount() const { return _unitList.size(); }
    AudioUnit * getUnitById(ID id); //for building track graph???
    const std::vector<std::unique_ptr<AudioUnit>> & getAllUnits() const { return _unitList; }

    Timeline & timeline() { return _timeline; }

    const std::vector<AudioRoute> & routes() const { return _routes; }
    void addRoute(AudioRoute route) { _routes.push_back(route); }
    void setRoute(AudioRoute route, std::size_t idx) { _routes.at(idx) = route; }
    void removeRoute(std::size_t idx) { _routes.erase(_routes.begin() + idx); }
    void removeRoutesForId(ID id);
    bool evaluateRoute(const AudioRoute & route);

    //TODO: use mixer not as separate unit, but as regular one
    Metronome * metronome() const;
    
    private:
    bool _isSolo;
    RenderPlan * _soloPlan;

    std::atomic<bool> _planInWork;
    RenderPlan * _renderPlan1;
    RenderPlan * _renderPlan2;
    
    Timeline _timeline;
    
    std::vector<std::unique_ptr<AudioUnit>> _unitList;
    std::vector<AudioRoute> _routes;

    std::unique_ptr<Metronome> _metronome;
    //std::unique_ptr<StepSequencer> _stepSequencer;
    //std::unique_ptr<ModulationEngine> _modEngine;
    //_globalParameterList??
};

}