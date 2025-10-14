// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/drivers/dummyDriver.h"

#include "core/primitives/AudioBuffer.h"
#include "core/utility/basicAudioManipulation.h"

#include <cstring>

namespace slr {

// static DriverRegistrar<DummyDriver>  __attribute__((used)) dummy_reg("Dummy Driver");

DummyDriver::DummyDriver() {

}

DummyDriver::~DummyDriver() {
    shutdown();
}

bool DummyDriver::init(frame_t sampleRate, frame_t bufferSize, int numInputs, int numOutputs) {
    _sampleRate = sampleRate;
    _bufferSize = bufferSize;
    _numInputs = numInputs;
    _numOutputs = numOutputs;

    sample_t ** datain = new sample_t*[numInputs];
    for(std::size_t i=0; i<numInputs; ++i) datain[i] = nullptr;

    _inputBuffers = new AudioBuffer(datain, numInputs, _bufferSize);

    for(int i=0; i<numInputs; ++i) {
        sample_t *in = static_cast<sample_t*>(new sample_t[bufferSize]);
        _inputBuffers->_data[i] = in;
        std::memset(in, 0.f, sizeof(sample_t) * _bufferSize);
    }

    sample_t ** dataout = new sample_t*[numOutputs];
    for(std::size_t i=0; i<numOutputs; ++i) dataout[i] = nullptr;

    _outputBuffers = new AudioBuffer(dataout, numOutputs, _bufferSize);

    for(int i=0; i<numOutputs; ++i) {
        sample_t *out = static_cast<sample_t*>(new sample_t[bufferSize]);
        _outputBuffers->_data[i] = out;
        std::memset(out, 0.f, sizeof(sample_t) * _bufferSize);
    }

    return true;
}

bool DummyDriver::start(AudioCallback clb) {
    _callback = clb;
    _keepRunning = true;
    _framesPassed = 0;

    _timerThread = std::thread([this]() {
        LOG_INFO("Dummy Start");
        ulong delayTime = 1'000'00 / this->_sampleRate;
        while(this->_keepRunning) {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            // std::this_thread::sleep_for(std::chrono::microseconds(delayTime));
            // std::this_thread::sleep_for(std::chrono::microseconds(200));
            this->process(this->bufferSize());
        }

        LOG_INFO("Dummy Stopped");
    });
    
    return true;
}

bool DummyDriver::stop() {
    _keepRunning = false;
    _timerThread.join();
    return true;
}

bool DummyDriver::shutdown() {
    if(_keepRunning) _keepRunning = false;
    if(_timerThread.joinable()) _timerThread.join();

    for(int i=0; i<_inputBuffers->_channels; ++i) {
        delete [] _inputBuffers->_data[i];
        _inputBuffers->_data[i] = nullptr;
    }
    delete [] _inputBuffers->_data;
    delete _inputBuffers;
    _inputBuffers = nullptr;

    for(int i=0; i<_outputBuffers->_channels; ++i) {
        delete [] _outputBuffers->_data[i];
        _outputBuffers->_data[i] = nullptr;
    }
    delete [] _outputBuffers->_data;
    delete _outputBuffers;
    _outputBuffers = nullptr;
    return true;
}

bool DummyDriver::restart() {
    return true;
}

bool DummyDriver::changeParameters(frame_t sampleRate, frame_t bufferSize, int numInputs, int numOutputs) {
    return true;
}

frame_t DummyDriver::process(frame_t frames) {
    clearAudioBuffers((*_inputBuffers)[0], (*_inputBuffers)[1], frames);
    clearAudioBuffers((*_outputBuffers)[0], (*_outputBuffers)[1], frames);

    //feed dummy data to inputs
    fillAudioBuffer((*_inputBuffers)[0], frames, 0.1f);
    fillAudioBuffer((*_inputBuffers)[1], frames, 0.2f);

    // (*_inputBuffers)[0][40] = 0.2f;
    // (*_inputBuffers)[1][40] = 0.8f;

    frame_t ret = _callback(_inputBuffers, _outputBuffers, frames, _framesPassed);
    _framesPassed += frames;
    
    return ret == frames ? 0 : ret;
}

}