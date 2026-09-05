// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/actions/ActionBase.h"
#include "common/defines.h"

#include <variant>
#include <string>

namespace slr {

struct ClipItem;
struct ClipItemView;
struct File;

namespace Actions {

struct LoadAsset : public ActionBase {
    LoadAsset() {}
    LoadAsset(const LoadAsset &rhs) :
        ActionBase(rhs),
        targetId(rhs.targetId),
        data(rhs.data) {}

    std::type_index actionType() const override { return typeid(LoadAsset); }

    ID targetId;
    std::variant<std::string, File*, ClipItem*> data;
};

}

}
