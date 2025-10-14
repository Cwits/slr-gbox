// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <cmath>
#include <string>
#include "defines.h"
// #include "core/ParameterArray.h"

namespace slr {

/*
track::track() {
    _volume = Parameter(id, name, defaultValue, minimal value, maxial value)
    _recArm = Parameter(id, name, default, min, max);
}

process() {
    float volume = _volume.get();
    bool recArm = _recArm.get();

}

Your parameter system (float-based) should be for:
    Gain, pan, pitch, filter freq, wet/dry, enable/disable
    UI-exposed modulateable controls
    Plugin parameters
    MIDI-mappable settings
    basically all parameters that could be automated and/or accessed from modulation engine, others should remain plain
It should not be used for:
    Transport positions
    Timeline locations
    Loop points
    Audio edits
*/

//not suitable for storing frame_t
class ParameterArray;
class ParameterBase {
    public:
    ParameterBase(std::string & name, float defaultValue, float minimalValue, float maximalValue);
    virtual ~ParameterBase() = default;

    virtual const float value() const = 0;
    virtual void setValue(float newValue) = 0;

    const ID & id() const { return _uniqueId; }
    const std::string & name() const { return _name; }
    const float & defaultValue() const { return _default; }
    const float & minimalValue() const { return _min; }
    const float & maximalValue() const { return _max; }

    protected:
    ID _uniqueId = 0; //set by ParameterArray
    std::string _name;
    float _default;
    float _min;
    float _max;

    friend class ParameterArray;
};

class ParameterFloat : public ParameterBase {
    public:
    ParameterFloat(std::string name, float defaultValue, float minimalValue, float maximalValue);
    ~ParameterFloat();

    operator float() const { return _value; }
    const float value() const override { return _value; }

    ParameterFloat& operator=(float value) { set(value); return *this; }
    void setValue(float value) override { set(value); }
    
    private:
    float _value;
    void set(float value) { _value = value; }
};

class ParameterInt : public ParameterBase {
    public:
    ParameterInt(std::string name, float defaultValue, float minimalValue, float maximalValue);
    ~ParameterInt();

    operator int() const { return _value; }
    const float value() const override { return static_cast<float>(_value); }

    ParameterInt& operator=(int value) { set(value); return *this; }
    void setValue(float value) override { set(std::round(value)); }

    private:
    int _value;
    void set(int value) { _value = value; }
};

class ParameterBool : public ParameterBase {
    public:
    ParameterBool(std::string name, float defaultValue = 0.f, float minimalValue = 0.f, float maximalValue = 1.f);
    ~ParameterBool();

    operator bool() const { return _value >= 0.5f; }
    const float value() const override { return _value >= 0.5f; }

    ParameterBool& operator=(bool value) { set(value); return *this; }
    void setValue(float value) { set(value >= 0.5f); }

    private:
    bool _value;
    void set(bool value) { _value = value; }
};

}