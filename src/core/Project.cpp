// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/Project.h"
#include "core/primitives/AudioUnit.h"
#include "core/Metronome.h"
#include "core/UnitManager.h"
#include "core/primitives/ControlContext.h"
#include "core/ControlEngine.h"
#include "logger.h"

#include <algorithm>

#define INITIAL_UNIT_SIZE 10

namespace slr {

Dependencies dummyDep = {
    .audio = nullptr,
    .audioDepsCnt = 0,
    .midi = nullptr,
    .midiDepsCnt = 0
};

RenderPlan dummyPlan = {
    .nodes = nullptr,
    .nodesCount = 0,
    .outputDeps = dummyDep
};

Project::Project() : _timeline(*this) {
    _unitList.reserve(INITIAL_UNIT_SIZE);
    _soloPlan = &dummyPlan;
    _renderPlan1 = &dummyPlan;
    _renderPlan2 = &dummyPlan;

    _metronome = std::make_unique<Metronome>();
    // _renderPlan1 = buildPlan(this);
    // _renderPlan2 = buildPlan(this);
    // _soloPlan = buildPlan(this);

    _isSolo = false;
    _planInWork = false;
}

Project::~Project() {
    destroyPlan(_soloPlan);
    destroyPlan(_renderPlan1);
    destroyPlan(_renderPlan2);

    BufferManager * man = ControlEngine::bufferManager();
    for(auto & unit : _unitList) {
        unit->destroy(man);
    }
}

AudioUnit * Project::createUnit(const ControlContext &ctx, const UnitDescriptor *desc, const ID forcedId) {
    AudioUnit * au = nullptr;
    try {

        slr::ID nextId = forcedId == 0 ? ctx.nextAudioUnitId() : forcedId;
        ClipContainerMap &map = _clipContainerMap; //.project->clipContainerMap();
        auto [it, inserted] = map.try_emplace(nextId);
        ClipContainerBuffer & storage = it->second;

        if(!inserted) {
            /*
                is this situation even possible? 
                i guess like... when there was attempt to create unit and it failed -> container wasn't deleted...
                just clear it and... ?
            */
            LOG_WARN("ClipStorage for id %d existed already. Checking if unit with similar id exists");
            AudioUnit * exists = getUnitById(nextId); //.project->getUnitById(nextId);
            if(exists) {
                //unit associated with this ID exists, can't touch that container.
                LOG_ERROR("Something really serious went off");
                LOG_ERROR("Expected next ID to be %d but it already taken", nextId);
                return nullptr;
            }
            LOG_WARN("Unit doesn't exist, safe to proceed, but may be some skew");
            storage.clear();
        }
        
        std::unique_ptr<AudioUnit> unit = desc->createRT(storage.inUseContainer(), nextId);
        au = unit.get();

        if(au->id() != nextId) {
            LOG_ERROR("Something went wrong... expected next unit id was %d but got %d", nextId, au->id());
            return nullptr;
        }

        if(!unit->create(ctx.bufferManager)) {
            LOG_ERROR("Failed to create unit for some reasons");
            return nullptr;
        }

        _unitList.push_back(std::move(unit)); //ctx.project->addUnit(std::move(unit));
    } catch(...) {
        LOG_ERROR("Failed to create module %s", desc->_name->data());
    }

    return au;
}

std::unique_ptr<AudioUnit> Project::removeUnit(ID id) {
    std::size_t size = _unitList.size();
    for(std::size_t i=0; i<size; ++i) {
        // if(_trackList.at(i) == nullptr) continue;
        if(_unitList.at(i).get()->id() == id) {
            // _unitList.erase(_unitList.begin()+i);
            std::unique_ptr<AudioUnit> ret = std::move(_unitList.at(i));
            _unitList.erase(_unitList.begin()+i);
            return std::move(ret);
            // return true;
        }
    }
    // return false;
    return nullptr;
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

bool Project::prepareSwappablePlan() {
    RenderPlan * newPlan = buildPlan(this);
				
	if(!newPlan) {
	    return false;
	}
	
    if(_planInWork.load(std::memory_order_acquire) == 0) {
        _renderPlan2 = newPlan;
    } else {
        _renderPlan1 = newPlan;
    }
    return true;
}

void Project::swapPlans() {
    _planInWork.fetch_xor(1, std::memory_order_release);
}

const RenderPlan * Project::editablePlan() const {
    const RenderPlan *ret = nullptr;
    if(_planInWork.load(std::memory_order_acquire) == 0) {
        ret = _renderPlan2;
    } else {
        ret = _renderPlan1;
    }
    return ret;
}

const RenderPlan * Project::runPlan() const {
    const RenderPlan *ret = nullptr;
    if(_planInWork.load(std::memory_order_acquire) == 0) {
        ret = _renderPlan1;
    } else {
        ret = _renderPlan2;
    }
    return ret;
}

const RenderPlan * Project::soloPlan() const {
    return _soloPlan;
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

ClipContainerBuffer & Project::getClipContainerBufferById(ID id) {
    return _clipContainerMap.at(id);
}

ClipItem * Project::findClipItemById(ID id) {
    return _clipStorage.findClipById(id);
}

}