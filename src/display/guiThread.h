// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <atomic>

void initGui();
void runGui(std::atomic<bool> &shutdown);

