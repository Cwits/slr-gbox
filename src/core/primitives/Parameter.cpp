// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/Parameter.h"

#include "common/Math.h"

namespace slr {

ParameterBase::ParameterBase(std::string & name, float defaultValue, float minimalValue, float maximalValue, float step) 
    : _name(name), _default(defaultValue), _min(minimalValue), _max(maximalValue), _step(step) {

}


/* 
    both append and remove can happen outside the rtcore? 
    in rt only copy-assign all ptrs and amt?
*/
void ParameterBase::appendModulation(const float * ptr, int ammount) { //ammount between 0 and 100
    if(ptr == nullptr) return;

    for(int i=0; i<8; ++i) {
        if(_mods[i].ptr == nullptr) {
            if(ammount > 100) ammount = 100;
            if(ammount < 0) ammount = 0;
            _mods[i].ptr = ptr;
            _mods[i].ammount = ammount;
            break;
        }
    }
} 

void ParameterBase::removeModulation(const float * ptr) {
    if(ptr == nullptr) return;
    
    int place = -1;
    for(int i=0; i<MAX_MODS; ++i) {
        if(_mods[i].ptr == ptr) {
            place = i;
        }
    }
    if(place == -1) return;

    for(int i=place; i<MAX_MODS; ++i) {
        if(i+1 != MAX_MODS && _mods[i+1].ptr != nullptr) {
            _mods[i].ptr = _mods[i+1].ptr;
            _mods[i].ammount = _mods[i+1].ammount;
        } else {
            _mods[i].ptr = nullptr;
            _mods[i].ammount = 0;
            break;
        }
    }
}

ParameterFloat::ParameterFloat(std::string name, float defaultValue, float minimalValue, float maximalValue, float step)
    : ParameterBase(name, defaultValue, minimalValue, maximalValue, step), _value(defaultValue) {

}

ParameterFloat::~ParameterFloat() {

}

float ParameterFloat::value(frame_t frame) const { 
    float modAmt = 0.0f;
    for(int i=0; i<8; ++i) {
        if(_mods[i].ptr == nullptr) continue;

        modAmt += ((_mods[i].ptr)[frame]) * _mods[i].ammount;
    }

    // float ret = _value + modAmt;
        
    return sMath::clamp(_value + modAmt, _min, _max);
}

ParameterInt::ParameterInt(std::string name, float defaultValue, float minimalValue, float maximalValue, float step)
    : ParameterBase(name, defaultValue, minimalValue, maximalValue, step), _value(defaultValue) {

}

ParameterInt::~ParameterInt() {

}

int ParameterInt::value(frame_t frame) const { 
    float modAmt = 0.0f;
    for(int i=0; i<8; ++i) {
        if(_mods[i].ptr == nullptr) continue;
    
        modAmt += ((_mods[i].ptr)[frame]) * _mods[i].ammount;
    }

    return _value + sMath::round(modAmt);
}

void ParameterInt::setValue(float value) { set(sMath::round(value)); }

ParameterBool::ParameterBool(std::string name, float defaultValue, float minimalValue, float maximalValue, float step)
    : ParameterBase(name, defaultValue, minimalValue, maximalValue, step), _value(defaultValue) {

}

ParameterBool::~ParameterBool() {

}

bool ParameterBool::value(frame_t frame) const { 
    float modAmt = 0.0f;
    for(int i=0; i<8; ++i) {
        if(_mods[i].ptr == nullptr) continue;

        modAmt += ((_mods[i].ptr)[frame]) * _mods[i].ammount;
    }
        
    //need to round with epsilon and some tolerance, otherwise always will be true
    //everything non zero must be true(either positive or negative)
    // return ((_value ? 1.f : 0.f) + modAmt) >= 0.5 ? true : false;
    return sMath::isApproximatelyZero(sMath::abs( (_value ? 1.f : 0.f) + modAmt ) );
}

void ParameterBool::setValue(float value) { set(sMath::isDefinitelyGreaterThan( sMath::abs(value), 0.5f) ); }

}