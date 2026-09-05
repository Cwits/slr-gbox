// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <string>
#include <nlohmann/json.hpp>

namespace slr {

// struct Project;
struct ControlContext;
struct ClipItem;
struct ClipItemView;
struct File;


namespace Serializer {
    
bool serialize(ControlContext &ctx);

nlohmann::ordered_json saveClip(const ClipItem *clip);
nlohmann::ordered_json saveClip(const ClipItemView *clip);
nlohmann::ordered_json saveFile(const File *file);

}


namespace Deserializer {

nlohmann::json loadJson(const std::string &path);

}


}