// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/RenderPlan.h"
#include "core/primitives/AudioRoute.h"
#include "core/primitives/MidiRoute.h"
#include "core/primitives/ClipItem.h"
#include "core/Timeline.h"
#include "common/defines.h"

#include <memory>

namespace slr {

struct AudioUnit;
struct Metronome;
struct UnitDescriptor;
struct ControlContext;
struct BufferManager;
struct StepSequencerEngine;
struct ModulationEngine;

class Project {
    public:
    Project();
    ~Project();

    void prepareForRecord();
    void prepareForPlay();
    void stopAfterRecord();
    void stopAfterPlay();

    const bool isSolo() const { return _isSolo; }

    bool prepareSwappablePlan();
    const RenderPlan * editablePlan() const;
    void swapPlans();
    const RenderPlan * runPlan() const;
    const RenderPlan * soloPlan() const;

    //nonRT
    // AudioUnit * createUnit(const ControlContext &ctx, const UnitDescriptor *desc, const ID forcedId);
    AudioUnit * createUnit(BufferManager * bmem, const UnitDescriptor *desc, const ID forcedId);
    std::unique_ptr<AudioUnit> removeUnit(ID id);
    void appendUnit(std::unique_ptr<AudioUnit> unit); //for delete undo??

    const std::size_t getUnitCount() const { return _unitList.size(); }
    AudioUnit * getUnitById(ID id); //for building track graph???
    const std::vector<std::unique_ptr<AudioUnit>> & getAllUnits() const { return _unitList; }

    ID getNextUnitId() const;

    const std::vector<AudioRoute> & routes() const { return _routes; }
    void addRoute(AudioRoute route) { _routes.push_back(route); }
    void setRoute(AudioRoute route, std::size_t idx) { _routes.at(idx) = route; }
    void removeRoute(std::size_t idx) { _routes.erase(_routes.begin() + idx); }
    void removeRoutesForId(ID id);
    bool evaluateRoute(const AudioRoute & route);
    bool unitHaveRoutes(ID unitId) const;

    const std::vector<MidiRoute> & midiRoutes() const { return _midiRoutes; }
    bool evaluateRoute(const MidiRoute & route) { return true; }
    void addRoute(MidiRoute route) { _midiRoutes.push_back(route); }
    void removeMidiRoute(std::size_t idx) { _midiRoutes.erase(_midiRoutes.begin() + idx); }

    Timeline & timeline() { return _timeline; }
    Metronome * metronome() const;

    ClipContainerBuffer & getClipContainerBufferById(ID id);
    ClipContainerMap & clipContainerMap() { return _clipContainerMap; }
    ClipStorage & clipStorage() { return _clipStorage; }
    
    ClipItem * findClipItemById(ID id);

    StepSequencerEngine * stepSequencer() const { return _stepSequencer.get(); }
    ModulationEngine * modulationEngine() const { return _modulationEngine.get(); }

    private:
    bool _isSolo;
    RenderPlan * _soloPlan;

    std::atomic<int> _planInWork;
    RenderPlan * _renderPlan1;
    RenderPlan * _renderPlan2;
    
    Timeline _timeline;

    ID _unitIDCounter;
    ID _clipIDCounter;
    ID _audioRouteIDCounter;
    ID _midiRouteIDCounter;
    ID _sequenceIDCounter;
    ID _modulationIDCounter;

    std::vector<std::unique_ptr<AudioUnit>> _unitList;
    std::vector<AudioRoute> _routes;
    std::vector<MidiRoute> _midiRoutes;

    std::unique_ptr<Metronome> _metronome;

    ClipContainerMap _clipContainerMap; ////TODO: this is a problem, because not cleared on unit deleting
    ClipStorage _clipStorage; 

    std::unique_ptr<StepSequencerEngine> _stepSequencer;
    std::unique_ptr<ModulationEngine> _modulationEngine;
    //std::unique_ptr<ModulationEngine> _modEngine;
    //_globalParameterList??

    
};

}