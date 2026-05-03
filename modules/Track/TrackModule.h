// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "core/ModuleManager.h"
#include "core/ActionsMap.h"

extern const slr::Module TrackModule;

namespace slr {
void registerTrackActions(std::map<std::type_index, CreatorFn> &map);
}