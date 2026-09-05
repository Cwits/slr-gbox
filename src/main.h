// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <atomic>

namespace slr {

static std::atomic<bool> _pendingShutdown;

}