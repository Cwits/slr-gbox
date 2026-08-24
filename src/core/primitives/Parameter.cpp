// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/Parameter.h"


namespace slr {

ParameterBase::ParameterBase(std::string & name, float defaultValue, float minimalValue, float maximalValue, float step) 
    : _name(name), _default(defaultValue), _min(minimalValue), _max(maximalValue), _step(step) {

}

ParameterFloat::ParameterFloat(std::string name, float defaultValue, float minimalValue, float maximalValue, float step)
    : ParameterBase(name, defaultValue, minimalValue, maximalValue, step), _value(defaultValue) {

}

ParameterFloat::~ParameterFloat() {

}



ParameterInt::ParameterInt(std::string name, float defaultValue, float minimalValue, float maximalValue, float step)
    : ParameterBase(name, defaultValue, minimalValue, maximalValue, step), _value(defaultValue) {

}

ParameterInt::~ParameterInt() {

}



ParameterBool::ParameterBool(std::string name, float defaultValue, float minimalValue, float maximalValue, float step)
    : ParameterBase(name, defaultValue, minimalValue, maximalValue, step), _value(defaultValue) {

}

ParameterBool::~ParameterBool() {

}


}