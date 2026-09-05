// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "snapshots/ModulationView.h"

#include "core/ModulationEngine.h"
#include "core/primitives/AudioUnit.h"

#include "common/logger.h"

#include <algorithm>

namespace slr {

ModulationPatternView::ModulationPatternView(const ModulationPattern* ptrn) :
    _ptrn(ptrn)
{
    update();
}

ModulationPatternView::~ModulationPatternView() {

}

ModulationShape ModulationPatternView::shape() const {
    return _ptrn->_shape;
}

ID ModulationPatternView::id() const {
    return _ptrn->_uniqueId;
}

void ModulationPatternView::update() {
    _shape = _ptrn->_shape;
    _amplitude = _ptrn->_amplitude;
    _phase = _ptrn->_phase;
    _rate = _ptrn->_lfoRate;
    _rateMode = _ptrn->_rateMode;
    _min = _ptrn->_valueMin;
    _max = _ptrn->_valueMax;

    for(int i=0; i<MAX_MODULATION_TARGETS; ++i) {
        _targets._raw[i] = _ptrn->_targets._raw[i];
    }

    incrementVersion();
}

ModulationEngineView::ModulationEngineView() {

}


std::shared_ptr<ModulationPatternView> ModulationEngineView::createModulationView(const ModulationPattern *ptrn) {
    std::shared_ptr<ModulationPatternView> ret = std::make_shared<ModulationPatternView>(ptrn);
    _storage.push_back(ret);

    return ret;
}

std::shared_ptr<ModulationPatternView> ModulationEngineView::findPatternById(ID id) {
    if(id == 0) return std::shared_ptr<ModulationPatternView>();
    
    auto found = std::find_if(
        _storage.begin(),
        _storage.end(),
        [id](const std::shared_ptr<ModulationPatternView> & ptrn) {
            return id == ptrn->id();
        }
    );

    if(found == _storage.end()) {
        return std::shared_ptr<ModulationPatternView>();
    }

    return *found;
}

}