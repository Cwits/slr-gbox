// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "core/UnitManager.h"
#include "core/ActionsMap.h"

extern const slr::UnitDescriptor TrackDescriptor;

namespace slr {
void registerTrackActions(std::map<std::type_index, CreatorFn> &map);
}