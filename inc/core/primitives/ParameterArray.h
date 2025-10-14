// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <vector>
#include "defines.h"
#include "slr_config.h"

namespace slr {

class ParameterBase;

struct ParameterArray {
    ParameterArray();
    ~ParameterArray();

    bool add(ParameterBase * base);
    const std::size_t count() const { return _list.size(); }

    ParameterBase * operator[](ID id);
    const ParameterBase * operator[](ID id) const;

    private:
    // int _count = 0;
    std::vector<ParameterBase*> _list;
};


}