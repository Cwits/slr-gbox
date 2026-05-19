// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <string>
#include <nlohmann/json.hpp>

namespace slr {

// struct Project;
struct ControlContext;

bool serialize(ControlContext &ctx);
// bool deserialize(std::string path);

namespace Deserializer {

nlohmann::json loadJson(const std::string &path);

}


}