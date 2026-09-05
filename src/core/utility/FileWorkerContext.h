// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "common/defines.h"

#include <string>
#include <functional>
#include <vector>
#include <memory>

namespace slr {

struct File;
struct FileWorker;


struct FileWorkerContext {
    FileWorkerContext(FileWorker *ptr, std::vector<std::unique_ptr<File>> &files);
    ~FileWorkerContext();

    FileWorker * const worker;
    std::vector<std::unique_ptr<File>> &files;
};

} //namespace slr
    