// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "AudioDriver.h"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string>
#include <thread>
#include <vector>

namespace slr {

class UDPDriver final : public AudioDriver {
public:
    UDPDriver();
    ~UDPDriver() override;

    bool init(frame_t sampleRate, frame_t bufferSize,
              int numInputs, int numOutputs) override;
    bool start(AudioCallback clb) override;
    bool stop() override;
    bool shutdown() override;
    bool restart() override;
    bool changeParameters(frame_t sampleRate, frame_t bufferSize,
                          int numInputs, int numOutputs) override;

    frame_t inputLatency(int port) const override;
    frame_t outputLatency(int port) const override;

private:
    void run();
    bool handshake();
    void closeSocket();

    int _socket = -1;
    uint32_t _streamId = 0;
    uint32_t _txSequence = 0;
    uint32_t _rxSequence = 0;
    std::atomic<bool> _threadRunning{false};
    std::thread _thread;

    std::vector<float> _txInterleaved;
    //ring buffer of received blocks: kRxRing blocks
    std::vector<float> _rxInterleaved;
    size_t _rxHead = 0; 
    size_t _rxCount = 0; 
};

}