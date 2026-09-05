// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <typeindex>

namespace slr {

struct ActionBase {
    ActionBase() {}
    ActionBase(const ActionBase& other) {}
	virtual ~ActionBase() = default;
    //int id() const { return _uniqueId; }

    virtual std::type_index actionType() const = 0;

    private:
    //int _uniqueId;
};

}