// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "core/filetasks/FileTask.h"
#include "common/defines.h"

#include <string>
#include <functional>
#include <optional>

namespace slr {

struct File;
struct FileWorkerContext;

namespace Tasks {

struct OpenFile : public FileTask {
    void exec(FileWorkerContext &ctx) override;

    std::string path;
    std::optional<ID> forcedId;
    std::optional<ID> viewForcedId;
    std::function<void(const File * file, bool success)> finished;
};

} //namespace Tasks

} //namespace slr
    