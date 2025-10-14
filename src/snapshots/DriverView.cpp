// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "snapshots/DriverView.h"
#include "core/drivers/AudioDriver.h"
#include "core/ControlEngine.h"

namespace slr {

DriverView::DriverView(AudioDriver * driver) {
    _sampleRate = driver->sampleRate();
    _bufferSize = driver->bufferSize();
    _inputs = driver->inputCount();
    _outputs = driver->outputCount();
}

DriverView::~DriverView() {

}

DriverView * DriverView::driverView() {
    return slr::ControlEngine::driverSnapshot();
}

}