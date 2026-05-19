// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

namespace slr {

struct FileWorkerContext;
struct Task {
    virtual ~Task() = default;
    virtual void exec(FileWorkerContext &ctx) = 0;
};

}