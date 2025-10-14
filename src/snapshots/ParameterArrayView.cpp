// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "snapshots/ParameterArrayView.h"
#include "snapshots/ParameterView.h"
#include "logger.h"

namespace slr {



ParameterArrayView::ParameterArrayView() {
}

ParameterArrayView::~ParameterArrayView() {
    for(ParameterBaseView * base : _list) {
        delete base;
    }
}

bool ParameterArrayView::add(ParameterBaseView * base) {
    _list.push_back(base);
    return true;
}
    
ParameterBaseView * ParameterArrayView::operator[](ID id) {
    // if(id < 0 || id >= _count) {
    //     LOG_FATAL("Invalid parameter ID on []");
    //     return nullptr;
    // }
    return _list[id];
}

const ParameterBaseView * ParameterArrayView::operator[](ID id) const {
    // if(id < 0 || id >= _count) {
    //     LOG_FATAL("Invalid parameter ID on []");
    //     return nullptr;
    // }
    return _list[id];
}

    
}