// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <map>
#include <typeindex>
#include <memory>

namespace slr {

struct ActionExecutable;
struct ActionBase;

using CreatorFn = std::unique_ptr<ActionExecutable>(*)(const ActionBase*);

void registerDefaultActions(std::map<std::type_index, CreatorFn> & map);
std::map<std::type_index, CreatorFn> & getActionMap();


}
