// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <string>
#include <math.h>
#include "common/defines.h"

namespace slr {

class ParameterBase;
class ParameterFloat;
class ParameterInt;
class ParameterBool;
struct ParameterFrame;

class ParameterBaseView {
    public:
    ParameterBaseView(const ParameterBase * base);
    virtual ~ParameterBaseView() = default;

    virtual const float value() const = 0;
    virtual void setValue(float newValue) = 0;

    ID id() const;
    const std::string & name() const;
    float defaultValue() const;
    float minimalValue() const;
    float maximalValue() const;

    private:
    const ParameterBase * const _base;
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

struct ParameterFrameView : public ParameterBaseView {
    ParameterFrameView(const ParameterFrame *);
    ~ParameterFrameView();

    operator frame_t() const { return _value; }
    const float value() const override { return normalize(_value); }

    ParameterFrameView& operator=(frame_t f) { set(f); return *this; }
    void setValue(float value) override { set(static_cast<frame_t>(value)); } //??

    private:
    float normalize(frame_t val) const;

    frame_t _value;
    void set(frame_t value) { _value = value; }
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