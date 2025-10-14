// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "defines.h"
#include "slr_config.h"

#include <vector>

namespace slr {

class ParameterBaseView;
struct ParameterArrayView {
    ParameterArrayView();
    ~ParameterArrayView();

    bool add(ParameterBaseView * base);
    const std::size_t count() const { return _list.size(); }

    ParameterBaseView * operator[](ID id);
    const ParameterBaseView * operator[](ID id) const;

    private:
    std::vector<ParameterBaseView*> _list;
};

}