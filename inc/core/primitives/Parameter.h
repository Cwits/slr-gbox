// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <cmath>
#include <string>
#include <vector>
#include <array>

#include "defines.h"
#include "common/Math.h"

namespace slr {

class ParameterArray;
class ParameterBase {
    public:
    ParameterBase(std::string & name, float defaultValue, float minimalValue, float maximalValue, float step);
    virtual ~ParameterBase() = default;

    virtual float value() const = 0; //returns pure value, without modulations
    virtual void setValue(float newValue) = 0;

    ID id() const { return _uniqueId; }
    const std::string & name() const { return _name; }
    float defaultValue() const { return _default; }
    float minimalValue() const { return _min; }
    float maximalValue() const { return _max; }

    virtual float normalized() { return 0.0f; }

    /* 
        both append and remove can happen outside the rtcore? 
        in rt only copy-assign all ptrs and amt?
    */
    void appendModulation(const float * ptr, int ammount) { //ammount between 0 and 100
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

    void removeModulation(const float * ptr) {
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

    protected:
    ID _uniqueId = 0; //set by ParameterArray
    std::string _name;
    float _default;
    float _min;
    float _max;
    float _step;

    struct ModulationPtr {
        const float * ptr; // -> ptr to buffer of normalized(?) values, buffer size is equal blockSize
        int ammount; 
    };

    static const int MAX_MODS = 8;
    std::array<ModulationPtr, MAX_MODS> _mods;

    friend class ParameterArray;
};

class ParameterFloat : public ParameterBase {
    public:
    ParameterFloat(std::string name, float defaultValue, float minimalValue, float maximalValue, float step);
    ~ParameterFloat();

    operator float() const { return _value; }
    float value() const override { return _value; }
    
    float value(frame_t frame) const { 
        float modAmt = 0.0f;
        for(int i=0; i<8; ++i) {
            if(_mods[i].ptr == nullptr) continue;

            modAmt += ((_mods[i].ptr)[frame]) * _mods[i].ammount;
        }

        // float ret = _value + modAmt;
        
        return sMath::clamp(_value + modAmt, _min, _max);
    }

    ParameterFloat& operator=(float value) { set(value); return *this; }
    void setValue(float value) override { set(value); }
    
    private:
    float _value;
    void set(float value) { _value = value; }
};

class ParameterInt : public ParameterBase {
    public:
    ParameterInt(std::string name, float defaultValue, float minimalValue, float maximalValue, float step);
    ~ParameterInt();

    operator int() const { return _value; }
    float value() const override { return static_cast<float>(_value); }

    int value(frame_t frame) const { 
        float modAmt = 0.0f;
        for(int i=0; i<8; ++i) {
            if(_mods[i].ptr == nullptr) continue;

            modAmt += ((_mods[i].ptr)[frame]) * _mods[i].ammount;
        }

        return _value + sMath::round(modAmt);
    }

    ParameterInt& operator=(int value) { set(value); return *this; }
    void setValue(float value) override { set(sMath::round(value)); }

    private:
    int _value;
    void set(int value) { _value = value; }
};

class ParameterBool : public ParameterBase {
    public:
    ParameterBool(std::string name, float defaultValue = 0.f, float minimalValue = 0.f, float maximalValue = 1.f, float step = 1.f);
    ~ParameterBool();

    operator bool() const { return _value; }
    float value() const override { return _value ? 1.0f : 0.0f; }

    bool value(frame_t frame) const { 
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

    ParameterBool& operator=(bool value) { set(value); return *this; }
    void setValue(float value) override { set(sMath::isDefinitelyGreaterThan( sMath::abs(value), 0.5f) ); }

    private:
    bool _value;
    void set(bool value) { _value = value; }
};

//huh?
/*
struct ParameterEnum : public ParameterBase {
    ParameterEnum(std::string name, std::vector<std::string> variants, float defaultValue = 0.f, float minimalValue = 0.f, float maximalValue = 999.f, float step = 1.f) {

    }

    float value() const { return _value; }
    void setValue(float value) override {}

    private:
    float _value;
    void set(float value) { _value = value; }

    std::vector<std::string> _variants;
};
*/
}