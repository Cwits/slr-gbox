// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/drivers/AudioDriver.h"
#include "core/primitives/AudioBuffer.h"

#include <thread>
#include <atomic>

namespace slr {

class DummyDriver : public AudioDriver {
    public:
    DummyDriver();
    virtual ~DummyDriver();
    bool init(frame_t sampleRate, frame_t bufferSize, int numInputs, int numOutputs) override;
    bool start(AudioCallback clb) override;
    bool stop() override;
    bool shutdown() override;
    bool restart() override;
    bool changeParameters(frame_t sampleRate, frame_t bufferSize, int numInputs, int numOutputs) override;

    const frame_t inputLatency(int port) override { return 64; }
    const frame_t outputLatency(int port) override { return 128; }

    private:
    std::thread _timerThread;
    std::atomic<bool> _keepRunning;

    AudioBuffer * _inputBuffers;
    AudioBuffer * _outputBuffers;

    frame_t process(frame_t frames);
};

}