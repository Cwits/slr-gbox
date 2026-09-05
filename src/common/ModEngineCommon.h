// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "common/defines.h"
#include <array>

namespace slr {

enum class ModulationShape {
    sin,
    saw,
    square,
    tri,
    rnd
};

enum class ModulationTargetType {
    ERROR,
    unit,
    modulation, 
    stepSequence
};

struct ModulationTarget {
    ModulationTargetType type;
    ID targetID;
    ID parameterID;

    ModulationTarget & operator=(const ModulationTarget &other) {
        type = other.type;
        targetID = other.targetID;
        parameterID = other.parameterID;
        return *this;
    }

    bool isValid() const { 
        return (type != ModulationTargetType::ERROR && targetID != 0);
    }
};

static const int MAX_MODULATION_TARGETS = 32;

struct ModTargetArray {
    ModTargetArray() { clearArray(); }
    ModTargetArray(const ModTargetArray &rhs) {
        for(int i=0; i<MAX_MODULATION_TARGETS; ++i) {
            _raw[i] = rhs._raw[i];
        }
    }

    //must check that unit and parameter exists
    bool addTarget(ModulationTargetType type, ID targetID, ID paramID) {
        bool ret = false;
        for(int i=0; i<MAX_MODULATION_TARGETS; ++i) {
            if(_raw[i].type == ModulationTargetType::ERROR && _raw[i].targetID == 0) {
                _raw[i].type = type;
                _raw[i].targetID = targetID;
                _raw[i].parameterID = paramID;
                ret = true;
                break;
            }
        }
        return ret;
    }

    bool removeTarget(ModulationTargetType type, ID targetID, ID paramID) {
        if(targetID == 0) return false;

        int pos = -1;
        for(int i=0; i<MAX_MODULATION_TARGETS; ++i) {
            if(_raw[i].type == type && _raw[i].targetID == targetID && _raw[i].parameterID == paramID) {
                pos = i;
            }
        }

        //failed, no such target
        if(pos == -1) return false;

        for(int i=pos; i<MAX_MODULATION_TARGETS; ++i) {
            if(i+1 != MAX_MODULATION_TARGETS && _raw[i+1].targetID != 0) {
                _raw[i].type = _raw[i+1].type;
                _raw[i].targetID      = _raw[i+1].targetID;
                _raw[i].parameterID = _raw[i+1].parameterID;
            } else {
                _raw[i].type = ModulationTargetType::ERROR;
                _raw[i].targetID = 0;
                _raw[i].parameterID = 0;
                break;
            }
        }

        return true;
    }

    bool canAddTarget() const { 
        for(int i=0; i<MAX_MODULATION_TARGETS; ++i) {
            if(_raw[i].type == ModulationTargetType::ERROR && _raw[i].targetID == 0) return true;
        }
        return false;
    }

    void clearArray() {
        for(int i=0; i<MAX_MODULATION_TARGETS; ++i) {
            _raw[i].type = ModulationTargetType::ERROR;
            _raw[i].targetID = 0;
            _raw[i].parameterID = 0;
        }
    }

    int maxTargets() const { return MAX_MODULATION_TARGETS; }

    bool haveTargets() const { 
        for(int i=0; i<MAX_MODULATION_TARGETS; ++i) {
            if(_raw[i].type != ModulationTargetType::ERROR &&
                _raw[i].targetID != 0) return true;
        }
        return false;
    }

    std::array<ModulationTarget, MAX_MODULATION_TARGETS> _raw;
};


}