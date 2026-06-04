// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "core/UnitManager.h"
#include "core/ActionsMap.h"

extern const slr::UnitDescriptor SamplerDescriptor;

namespace slr {
void registerSamplerActions(std::map<std::type_index, CreatorFn> &map);
}