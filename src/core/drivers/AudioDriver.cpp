// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/drivers/AudioDriver.h"
#include "core/drivers/jackDriver.h"
#include "core/drivers/dummyDriver.h"
#include "core/drivers/udpDriver.h"

namespace slr { 

static DriverRegistrar<DummyDriver>  __attribute__((used)) dummy_reg("Dummy Driver");
static DriverRegistrar<JackDriver>  __attribute__((used)) jack_reg("Jack Driver");
static DriverRegistrar<UDPDriver> __attribute__((used)) udp_reg("UDP Driver");

AudioDriver::AudioDriver() : 
    _sampleRate(0), 
    _bufferSize(0), 
    _numInputs(0),
    _numOutputs(0),
    _isRunning(false) {

}

AudioDriver::~AudioDriver() {

}

void AudioDriverFactory::register_driver(const std::string& name, Creator create) {
    instance().registry[name] = std::move(create);  
}

}