// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/ParameterArray.h"
#include "logger.h"
#include "core/primitives/Parameter.h"


namespace slr {

ParameterArray::ParameterArray() {
}

ParameterArray::~ParameterArray() {
    for(ParameterBase * base : _list) {
        delete base;
    }
}

bool ParameterArray::add(ParameterBase * base) {
    base->_uniqueId = static_cast<int>(_list.size());
    _list.push_back(base);
    return true;
}
    
ParameterBase * ParameterArray::operator[](ID id) {
    // if(id < 0 || id >= _list.size()) {
    //     LOG_FATAL("Invalid parameter ID on []");
    //     return nullptr;
    // }
    return _list[id];
}

const ParameterBase * ParameterArray::operator[](ID id) const {
    // if(id < 0 || id >= _list.size()) {
    //     LOG_FATAL("Invalid parameter ID on []");
    //     return nullptr;
    // }
    return _list[id];
}


}