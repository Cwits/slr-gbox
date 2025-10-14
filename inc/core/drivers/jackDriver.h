// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/drivers/AudioDriver.h"
#include "core/primitives/AudioBuffer.h"
#include <jack/jack.h>
#include <string>
#include <vector>
#include <thread>

namespace slr {

class JackDriver : public AudioDriver {
    public:
    JackDriver();
    virtual ~JackDriver();
    bool init(frame_t sampleRate, frame_t bufferSize, int numInputs, int numOutputs) override;
    bool start(AudioCallback clb) override;
    bool stop() override;
    bool shutdown() override;
    bool restart() override;
    bool changeParameters(frame_t sampleRate, frame_t bufferSize, int numInputs, int numOutputs);

    const bool autoconnect() const { return _autoconnect; }
    void setAutoconnect(bool val) { _autoconnect = val; }
    
    const frame_t inputLatency(int port) override;
    const frame_t outputLatency(int port) override;

    private:
    // std::thread _jackServerThread;
    std::string _clientName;
    jack_client_t * _client;
    std::vector<jack_port_t*> _inputs;
    std::vector<jack_port_t*> _outputs;
    bool _autoconnect;

    AudioBuffer * _inputBuffers;
    AudioBuffer * _outputBuffers;
    
    static int jack_process(jack_nframes_t nframes, void *arg);
    static int xrun_callback(void *arg);
    static int graph_order_callback(void *arg);

    bool createPorts(int numIn, int numOut);
    bool destroyPorts();
    bool connect();
};

}