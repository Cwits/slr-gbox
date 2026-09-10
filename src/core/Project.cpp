// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/Project.h"

#include "core/primitives/AudioUnit.h"
#include "core/utility/ControlContext.h"

#include "core/Metronome.h"
#include "core/UnitManager.h"
#include "core/ControlEngine.h"
#include "core/StepSequencer.h"
#include "core/ModulationEngine.h"
#include "core/RenderPlan.h"
#include "core/RtEngine.h"
#include "core/SettingsManager.h"
#include "core/drivers/AudioDriver.h"

#include "common/logger.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <queue>

#define INITIAL_UNIT_SIZE 10

namespace slr {

Project::Project() : _timeline(*this) {
    _unitList.reserve(INITIAL_UNIT_SIZE);
    
    _metronome = std::make_unique<Metronome>();
    _stepSequencer = std::make_unique<StepSequencerEngine>();
    _modulationEngine = std::make_unique<ModulationEngine>();

    std::unique_ptr<PlanBuilder::PlanHolder> p1 = std::make_unique<PlanBuilder::PlanHolder>();
    std::unique_ptr<PlanBuilder::PlanHolder> p2 = std::make_unique<PlanBuilder::PlanHolder>();
    
    p1->plan = std::make_unique<RenderPlan>();
    p1->nodes.reserve(20);
    p1->nodeDepHolder.reserve(20);
    p1->modPatterns.reserve(10);
    p1->sequences.reserve(10);
    
    p2->plan = std::make_unique<RenderPlan>();
    p2->nodes.reserve(20);
    p2->nodeDepHolder.reserve(20);
    p2->modPatterns.reserve(10);
    p2->sequences.reserve(10);

    _plans.init(p1, p2);

    _unitIDCounter = 0;
    _clipIDCounter = 0;
    _audioRouteIDCounter = 0;
    _midiRouteIDCounter = 0;
    _sequenceIDCounter = 0;
    _modulationIDCounter = 0;
}

Project::~Project() {
    // destroyPlan(_soloPlan);
    // destroyPlan(_renderPlan1);
    // destroyPlan(_renderPlan2);

    BufferManager * man = ControlEngine::bufferManager();
    for(auto & unit : _unitList) {
        unit->destroy(man);
    }
}

// AudioUnit * Project::createUnit(const ControlContext &ctx, const UnitDescriptor *desc, const ID forcedId) {
AudioUnit * Project::createUnit(BufferManager * bmem, const UnitDescriptor *desc, const ID forcedId) {
    AudioUnit * au = nullptr;
    try {

        // slr::ID nextId = forcedId == 0 ? ctx.nextAudioUnitId() : forcedId;
        ID nextId = forcedId;
        ClipContainerMap &map = _clipContainerMap; //.project->clipContainerMap();
        auto [it, inserted] = map.try_emplace(nextId);
        ContainerBuffer & storage = it->second;

        if(!inserted) {
            /*
                is this situation even possible? 
                i guess like... when there was attempt to create unit and it failed -> container wasn't deleted...
                just clear it and... ?
            */
            LOG_WARN("ClipStorage for id %d existed already. Checking if unit with similar id exists", nextId);
            AudioUnit * exists = getUnitById(nextId); //.project->getUnitById(nextId);
            if(exists) {
                //unit associated with this ID exists, can't touch that container.
                LOG_ERROR("Something really serious went off");
                LOG_ERROR("Expected next ID to be %d but it already taken", nextId);
                return nullptr;
            }
            LOG_WARN("Unit doesn't exist, safe to proceed, but may be some skew");
            // storage.clear();
            //need to clear both buffers somehow
        } else {
            auto b1 = std::make_unique<ClipContainer>();
            auto b2 = std::make_unique<ClipContainer>();
            b1->reserve(2);
            b2->reserve(2);
            storage.init(b1, b2);
        }
        
        std::unique_ptr<AudioUnit> unit = desc->createRT(storage.readable().get(), nextId);
        au = unit.get();

        if(au->id() != nextId) {
            LOG_ERROR("Something went wrong... expected next unit id was %d but got %d", nextId, au->id());
            return nullptr;
        }

        if(!unit->create(bmem)) {
            LOG_ERROR("Failed to create unit for some reasons");
            return nullptr;
        }

        _unitList.push_back(std::move(unit)); //ctx.project->addUnit(std::move(unit));
    } catch(...) {
        LOG_ERROR("Failed to create module %s", desc->_name->data());
    }

    _unitIDCounter = std::max(_unitIDCounter+1, forcedId);
    return au;
}

std::unique_ptr<AudioUnit> Project::removeUnit(ID id) {
    auto it = std::find_if(
        _unitList.begin(),
        _unitList.end(),
        [id](const std::unique_ptr<AudioUnit> &p) {
            return id == p->id();
        }
    );

    if(it == _unitList.end()) {
        LOG_ERROR("No unit with id %u found", id);
        return std::unique_ptr<AudioUnit>();
    }

    std::unique_ptr<AudioUnit> ret = std::move(*it);
    _unitList.erase(it);
    return std::move(ret);
}

void Project::appendUnit(std::unique_ptr<AudioUnit> unit) { //for delete undo??
    _unitList.push_back(std::move(unit));
}

AudioUnit * Project::getUnitById(ID id) {
    if(id == 0) return nullptr; //id == 0 is metronome
    std::size_t size = _unitList.size();
    for(std::size_t i=0; i<size; ++i) {
        if(_unitList.at(i).get()->id() == id) return _unitList.at(i).get();
    }

    LOG_ERROR("Wrong Unit ID");
    return nullptr;
}

ID Project::getNextUnitId() const {
    return _unitIDCounter+1;
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
    for(auto &s : _stepSequencer->notToUseSequences()) {
        s->prepareToPlay();
    }
    for(auto &p : _modulationEngine->notToUsePatterns()) {
        p->prepareToPlay();
    }
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
    for(auto &s : _stepSequencer->notToUseSequences()) {
        s->stopAfterPlay();
    }
    for(auto &p : _modulationEngine->notToUsePatterns()) {
        p->stopPlaying();
    }
}

bool Project::evaluateRoute(const AudioRoute & route) {
    LOG_WARN("Routing evaluation not implemented yet");
    return true;
}

bool Project::unitHaveRoutes(ID unitId) const {
    for(auto &ar : _routes) {
        if(ar._sourceType == slr::AudioRoute::Type::INT && ar._sourceId == unitId) return true;
        if(ar._targetType == slr::AudioRoute::Type::INT && ar._targetId == unitId) return true;
    }

    for(auto &mr : _midiRoutes) {
        if(mr._sourceType == slr::MidiRoute::Type::INT && mr._sourceId == unitId) return true;
        if(mr._targetType == slr::MidiRoute::Type::INT && mr._targetId == unitId) return true;
    }

    return false;
}

void Project::removeRoutesForId(ID id) {
    _routes.erase(std::remove_if(_routes.begin(), _routes.end(), 
        [&id](const AudioRoute &r) {
            if((r._sourceId == id && r._sourceType == AudioRoute::Type::INT) || 
                (r._targetId == id && r._targetType == AudioRoute::Type::INT)) {
                return true;
            } 
            
            return false;
    }), _routes.end());

    _midiRoutes.erase(std::remove_if(_midiRoutes.begin(), _midiRoutes.end(),
        [&id](const MidiRoute &r) {
            if((r._sourceId == id && r._sourceType == MidiRoute::Type::INT) ||
                r._targetId == id && r._targetType == MidiRoute::Type::INT) {
                return true;
            }

            return false;
    }), _midiRoutes.end());
}

ContainerBuffer & Project::getClipContainerBufferById(ID id) {
    return _clipContainerMap.at(id);
}

ClipItem * Project::findClipItemById(ID id) {
    return _clipStorage.findClipById(id);
}

const RenderPlan * Project::getSwappablePlan(ControlContext &ctx, uint16_t bitmask) {
    using namespace PlanBuilder;

    PlanHolder * writable = _plans.writable().get();
    const PlanHolder * readable = _plans.readable().get();
    clearHolder(writable);

    writable->plan->metro = metronome();
    writable->plan->timeline = &timeline();

    bool success = true;
    if(bitmask & (uint16_t)PlanRebuild::All) {
        success &= buildUnits(ctx, writable);
        success &= buildSequences(ctx, writable);
        success &= buildModulations(ctx, writable);
    } else if(bitmask & (uint16_t)PlanRebuild::Units) {
        success &= buildUnits(ctx, writable);
        success &= copySequences(writable, readable);
        success &= copyModulations(writable, readable);
    } else if(bitmask & (uint16_t)PlanRebuild::Sequences) {
        success &= copyUnits(writable, readable);
        success &= buildSequences(ctx, writable);
        success &= copyModulations(writable, readable);
    } else if(bitmask & (uint16_t)PlanRebuild::Modulations) {
        success &= copyUnits(writable, readable);
        success &= copySequences(writable, readable);
        success &= buildModulations(ctx, writable);
    }

    if(success) return writable->plan.get();
    else return nullptr;
}

}