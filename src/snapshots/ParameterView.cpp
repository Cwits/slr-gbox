// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "snapshots/ParameterView.h"
#include "core/primitives/Parameter.h"

namespace slr {


ParameterBaseView::ParameterBaseView(ParameterBase * base) 
    : _name(base->name()), 
    _uniqueId(base->id()), 
    _default(base->defaultValue()), 
    _min(base->minimalValue()), 
    _max(base->maximalValue())
{

}

ParameterFloatView::ParameterFloatView(ParameterFloat * par) : ParameterBaseView(par), _value(par->value()) {
 
}

ParameterFloatView::~ParameterFloatView() {

}

ParameterIntView::ParameterIntView(ParameterInt * par) : ParameterBaseView(par), _value(par->value()) {

}

ParameterIntView::~ParameterIntView() {

}

ParameterBoolView::ParameterBoolView(ParameterBool * par) : ParameterBaseView(par), _value(par->value()) {

}

ParameterBoolView::~ParameterBoolView() {

}


    
}