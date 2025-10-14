// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/drivers/jackDriver.h"
#include "logger.h"

#include "core/primitives/AudioBuffer.h"
#include "core/utility/basicAudioManipulation.h"

#include <cstdlib>
#include <cstdio>
#include <thread>
#include <chrono>
#include <string>
#include <atomic>
#include <condition_variable>

static std::atomic<bool> _ready;
static std::atomic<bool> _result;
/*
TODO: handle xruns, shutdown, errors? smth else?
*/
namespace slr {

// static DriverRegistrar<JackDriver>  __attribute__((used)) jack_reg("Jack Driver");

int JackDriver::jack_process(jack_nframes_t nframes, void *arg) {
    JackDriver * dri = static_cast<JackDriver*>(arg);

    std::size_t size = dri->_inputs.size();
    for(std::size_t i=0; i<size; ++i) {
        sample_t * in = static_cast<sample_t*>(jack_port_get_buffer(dri->_inputs.at(i), dri->_bufferSize));
        dri->_inputBuffers->_data[i] = in;
    }
    size = dri->_outputs.size();
    for(std::size_t i=0; i<size; ++i) {
        sample_t * out = static_cast<sample_t*>(jack_port_get_buffer(dri->_outputs.at(i), dri->_bufferSize));
        dri->_outputBuffers->_data[i] = out;
    }

    clearAudioBuffers(dri->_outputBuffers->_data[0], 
                        dri->_outputBuffers->_data[1],
                        static_cast<frame_t>(nframes));

    frame_t ret = dri->_callback(dri->_inputBuffers, dri->_outputBuffers, static_cast<frame_t>(nframes), dri->_framesPassed); 
    dri->_framesPassed += static_cast<frame_t>(nframes);

    //how to get outputs?

    return (ret == static_cast<frame_t>(nframes)) ? 0 : static_cast<int>(ret); //return 0 on success
}

JackDriver::JackDriver() : _client(nullptr), _autoconnect(true) {
}

JackDriver::~JackDriver() {
    shutdown();
}

bool JackDriver::init(frame_t sampleRate, frame_t bufferSize, int numInputs, int numOutputs) {
  // std::string jackParameters = "/usr/bin/jackd -T -R -P 95 -dalsa -dhw:0 -r" + std::to_string(engine->_sampleRate) + " -p" + std::to_string(engine->_bufferSize) + " -n2 -Xseq -D -Phw:sndrpihifiberry -i2 -o2";
    bool ret = true;
    std::string jackParameters = "/usr/bin/jackd -T -R -P 95 -dalsa -dhw:2 -r" + 
                std::to_string(sampleRate) + " -p" + 
                std::to_string(bufferSize) +  
                " -n2 -Xnone -D -Phw:slraudio,0 -Chw:slraudio,1 -i" + 
                std::to_string(numInputs) + 
                " -o" + 
                std::to_string(numOutputs);  

    std::thread([jackParameters]() {
        FILE * pipe = popen(jackParameters.c_str(), "r");
        if(!pipe) { _result = false; _ready = true; return; }

        char buffer[256] = {0};
        while (fgets(buffer, sizeof(buffer), pipe)) {
            std::string line(buffer);
            std::cout << line;
            if (line.find("ALSA: use 2 periods for playb") != std::string::npos) {
                _result = true;
                _ready = true;
                break;
            }

            if (line.find("`default' server already active") != std::string::npos) {
                _result = true;
                _ready = true;
                break;
            }

            if(line.find("JACK server is not running or cannot be started") != std::string::npos) {
                _result = false;
                _ready = true;
                break;
            }
        }

        pclose(pipe);

    }).detach();

    while(!_ready) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if (!_result) {
        ret = false;
        LOG_ERROR("Failed to start Jackd server");
    }

    if(ret) {
        _clientName = "Sample.Loop.Repeat";
        _client = jack_client_open(_clientName.c_str(),
                                    JackNullOption, 0, 0);

        if(!_client) ret = false;
    }

    if(ret) {
        if(!createPorts(numInputs, numOutputs)) ret = false;
        _numInputs = numInputs;
        _numOutputs = numOutputs;
    }

    if(ret) {
        if(jack_set_process_callback(_client, JackDriver::jack_process, (void*)this) ) 
            ret = false;

    }

    jack_set_xrun_callback(_client, &JackDriver::xrun_callback, static_cast<void*>(this));
    jack_set_graph_order_callback(_client, &JackDriver::graph_order_callback, static_cast<void*>(this));
    
    if(ret) {
        _sampleRate = sampleRate;
        _bufferSize = bufferSize;
        LOG_INFO("Jack Driver initialized successfully!");
    }

    return ret;
}

bool JackDriver::start(AudioCallback clb) {
    bool ret = true;

    _framesPassed = 0;
    _callback = clb;
    
    
    if(jack_activate(_client)) {
        ret = false;
    }

    if(ret && _autoconnect) {
        ret = connect();
    }

    if(!ret) LOG_ERROR("Error with activating jack client");
    else  {
        _isRunning = true;
        LOG_INFO("Jack activated successfully!");

        for(std::size_t i=0; i<_inputs.size(); ++i) {
            jack_latency_range_t lat = {0, 0};
            jack_port_get_latency_range(_inputs.at(i), JackCaptureLatency, &lat);
            LOG_INFO("Input latency range for port %d is min:%u max:%u", 
                        static_cast<int>(i), lat.min, lat.max);

            if(lat.min != _bufferSize || lat.max != _bufferSize) {
                LOG_WARN("Input latency range for port %d is different than expected! Min: %u, max: %u, expected: %lu",
                                static_cast<int>(i), lat.min, lat.max, _bufferSize);
            }
        }

        
        for(std::size_t i=0; i<_outputs.size(); ++i) {
            jack_latency_range_t lat = {0, 0};
            jack_port_get_latency_range(_outputs.at(i), JackPlaybackLatency, &lat);
            LOG_INFO("Output latency range for port %d is min:%u max:%u", 
                        static_cast<int>(i), lat.min, lat.max);

            if(lat.min != (_bufferSize*2) || lat.max != (_bufferSize*2)) {  
                LOG_WARN("Output latency range for port %d is different than expected! Min: %u, max: %u, expected: %lu",
                                static_cast<int>(i), lat.min, lat.max, (_bufferSize*2));
            }
        }
    }
    
    return ret;
}

bool JackDriver::stop() {
    jack_deactivate(_client);
    _isRunning = false;
    return true;
}

bool JackDriver::shutdown() {
    bool ret = true;
    if(_isRunning) {
        ret = stop();
    }

    if(_inputs.size() != 0 || _outputs.size() != 0)
        ret = destroyPorts();

    if(_client) {
        if(jack_client_close(_client)) 
            ret = false;
    }
    //how to handle errors here???
    //_jackServerThread.join(); //   why it is not joining???
    return ret;
}

bool JackDriver::restart() {
    return true;
}

bool JackDriver::changeParameters(frame_t sampleRate, frame_t bufferSize, int numInputs, int numOutputs) {
    // _sampleRate = sampleRate;
    // _bufferSize = bufferSize;
    // _numInputs = numInputs;
    // _numOutputs = numOutputs;
    return true;
}

bool JackDriver::createPorts(int numIn, int numOut) {
    bool ret = true;
    for(int i=0; i<numIn; ++i) {
        std::string name = "SLR Input ";
        name.append(std::to_string(i+1));

        jack_port_t * in = jack_port_register(
            _client, 
            name.c_str(),
            JACK_DEFAULT_AUDIO_TYPE,
            JackPortIsInput,
            0
        );

        if(!in) {
            ret = false;
            LOG_FATAL("Failed to create Jack Input/Output ports!");
            break;
        }

        _inputs.push_back(in);
    }

    for(int i=0; i<numOut; ++i) {
        std::string name = "SLR Output ";
        name.append(std::to_string(i+1));

        jack_port_t * out = jack_port_register(
            _client, 
            name.c_str(),
            JACK_DEFAULT_AUDIO_TYPE,
            JackPortIsOutput,
            0
        );

        if(!out) {
            ret = false;
            LOG_FATAL("Failed to create Jack Input/Output ports!");
            break;
        }

        _outputs.push_back(out);
    }

    if(ret) LOG_INFO("Default ports created!");
    
    std::size_t size = _inputs.size();
    sample_t ** datain = new sample_t*[size];
    for(std::size_t i=0; i<size; ++i) datain[i] = nullptr;
    _inputBuffers = new AudioBuffer(datain, static_cast<uint8_t>(size), _bufferSize);
    
    size = _outputs.size();
    sample_t ** dataout = new sample_t*[size];
    for(std::size_t i=0; i<size; ++i) dataout[i] = nullptr;
    _outputBuffers = new AudioBuffer(dataout, static_cast<uint8_t>(size), _bufferSize);

    if(!_inputBuffers || !_outputBuffers) ret = false;
    
    if(ret == false) {
        LOG_WARN("Must delete everything!!!");
    }

    return ret;
}

bool JackDriver::destroyPorts() {
    bool ret = true;

    for(jack_port_t * port : _inputs) {
        if(!jack_port_unregister(_client, port)) {
            ret = false;
        }
    }

    delete [] _inputBuffers->_data;
    delete _inputBuffers;
    _inputBuffers = nullptr;
    _inputs.clear();

    for(jack_port_t * port : _outputs) {
        if(!jack_port_unregister(_client, port)) {
            ret = false;
        }
    }

    delete [] _outputBuffers->_data;
    delete _outputBuffers;
    _outputBuffers = nullptr;
    _outputs.clear();
    
    if(!ret) LOG_ERROR("Failed to destroy jack port/ports!");

    return ret;
}

bool JackDriver::connect() {
    bool ret = true;
    const char **port_names;
    
    port_names = jack_get_ports(_client, NULL, NULL, JackPortIsPhysical | JackPortIsInput);

    //applicatino outputs connect to jack playback
    if(!port_names[0] && !port_names[1]) {
        ret = false;
    } else {
        if( jack_connect(_client, jack_port_name(_outputs[0]), port_names[0]) ) ret = false;
        if( jack_connect(_client, jack_port_name(_outputs[1]), port_names[1]) ) ret = false;

        if(ret) LOG_INFO("Outputs connected successfully");
    }

    //jack capture to application inputs
    port_names = jack_get_ports(_client, NULL, NULL, JackPortIsPhysical | JackPortIsOutput);
    if(!port_names[0] && !port_names[1]) {
        ret = false;
    } else {
        if( jack_connect(_client, port_names[0], jack_port_name(_inputs[0])) ) ret = false;
        if( jack_connect(_client, port_names[1], jack_port_name(_inputs[1])) ) ret = false;

        if(ret) LOG_INFO("Inputs connected successfully!");
    }

    if(!ret) LOG_ERROR("Jack autoconnect ports failed!");
        
    return ret;
}

const frame_t JackDriver::inputLatency(int port) { 
    jack_latency_range_t lat = {0, 0};
    jack_port_get_latency_range(_inputs.at(port), JackCaptureLatency, &lat);
    return static_cast<frame_t>(lat.min);
}

const frame_t JackDriver::outputLatency(int port) { 
    jack_latency_range_t lat = {0, 0};
    jack_port_get_latency_range(_outputs.at(port), JackPlaybackLatency, &lat);
    return static_cast<frame_t>(lat.min);
}

int JackDriver::xrun_callback(void *arg) {
    JackDriver * dri = static_cast<JackDriver*>(arg);
    (void)dri;
    // dri->stop();
    LOG_WARN("XRUN");

    return 1;
}

int JackDriver::graph_order_callback(void *arg) {
    LOG_WARN("GRAPH ORDER");
    return 1;
}

}