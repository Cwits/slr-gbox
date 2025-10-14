// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"

namespace slr {

class AudioDriver;

struct DriverView {
    DriverView(AudioDriver * driver);
    ~DriverView();

    const frame_t sampleRate() const { return _sampleRate; }
    const frame_t bufferSize() const { return _bufferSize; }
    const int inputs() const { return _inputs; }
    const int outputs() const { return _outputs; }

    static DriverView * driverView();

    private:
    int _inputs;
    int _outputs;
    frame_t _sampleRate;
    frame_t _bufferSize;
};

}