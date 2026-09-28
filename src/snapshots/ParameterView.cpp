// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "snapshots/ParameterView.h"
#include "core/primitives/Parameter.h"

namespace slr {


ParameterBaseView::ParameterBaseView(const ParameterBase * base) 
    : _base(base)
{
}

ID ParameterBaseView::id() const { return _base->id(); }
const std::string & ParameterBaseView::name() const { return _base->name(); }
float ParameterBaseView::defaultValue() const { return _base->defaultValue(); }
float ParameterBaseView::minimalValue() const { return _base->minimalValue(); }
float ParameterBaseView::maximalValue() const { return _base->maximalValue(); }


ParameterFloatView::ParameterFloatView(ParameterFloat * par) : ParameterBaseView(par), _value(*par) {
 
}

ParameterFloatView::~ParameterFloatView() {

}

ParameterIntView::ParameterIntView(ParameterInt * par) : ParameterBaseView(par), _value(*par) {

}

ParameterIntView::~ParameterIntView() {

}

ParameterBoolView::ParameterBoolView(ParameterBool * par) : ParameterBaseView(par), _value(*par) {

}

ParameterBoolView::~ParameterBoolView() {

}


    
}