// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <functional>

namespace slr {
    class MidiPort;
}

namespace PushThread {

void start(slr::MidiPort * port);
void shutdown();
bool isRunning();

void postTask(std::function<void()> fn);

}
