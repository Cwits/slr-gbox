// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <string>
#include <math.h>
#include "defines.h"

namespace slr {

class ParameterBase;
class ParameterFloat;
class ParameterInt;
class ParameterBool;

class ParameterBaseView {
    public:
    ParameterBaseView(ParameterBase * base);
    virtual ~ParameterBaseView() = default;

    virtual const float value() const = 0;
    virtual void setValue(float newValue) = 0;

    const ID & id() const { return _uniqueId; }
    const std::string & name() const { return _name; }
    const float & defaultValue() const { return _default; }
    const float & minimalValue() const { return _min; }
    const float & maximalValue() const { return _max; }

    private:
    const std::string & _name;
    const ID & _uniqueId;
    const float & _default;
    const float & _min;
    const float & _max;
};

struct ParameterFloatView : public ParameterBaseView {
    public:
    ParameterFloatView(ParameterFloat * par);
    ~ParameterFloatView();

    operator float() const { return _value; }
    const float value() const override { return _value; }
    
    ParameterFloatView& operator=(float value) { set(value); return *this; }
    void setValue(float value) override { set(value); }

    private:
    float _value;
    void set(float value) { _value = value; }
};


class ParameterIntView : public ParameterBaseView {
    public:
    ParameterIntView(ParameterInt * par);
    ~ParameterIntView();

    operator int() const { return std::round(_value); }
    const float value() const override { return _value; }

    ParameterIntView& operator=(int value) { set(value); return *this; }
    void setValue(float value) override { set(value); }

    private:
    float _value;
    void set(int value) { _value = static_cast<float>(value); }
};

class ParameterBoolView : public ParameterBaseView {
    public:
    ParameterBoolView(ParameterBool * par);
    ~ParameterBoolView();

    operator bool() const { return _value >= 0.5; }
    const float value() const override { return _value; }

    ParameterBoolView& operator=(bool value) { set(value); return *this; }
    void setValue(float value) { set(value); }

    private:
    float _value;
    void set(bool value) { _value = (value > 0.5f) ? 1.f : 0.f; }
};

}