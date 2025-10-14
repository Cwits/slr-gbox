// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

namespace slr {

class FileWorker;
struct Task {
    virtual ~Task() = default;
    virtual void exec(FileWorker * f) = 0;
};

}