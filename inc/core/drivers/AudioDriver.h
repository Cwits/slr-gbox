// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "logger.h"
#include "defines.h"
#include "core/primitives/AudioBuffer.h"

#include <functional>
#include <unordered_map>
#include <memory>

//some dependencie that i don't understand...
//without it AudioDriver won't compile...
#include <iostream>
namespace slr {

// class AudioBuffer;

class AudioDriver {
    public:
    using AudioCallback = std::function<frame_t(AudioBuffer*, AudioBuffer*, frame_t, frame_t)>;

    AudioDriver();
    virtual ~AudioDriver();

    virtual bool init(frame_t sampleRate, frame_t bufferSize, int numInputs, int numOutputs) = 0;
    virtual bool start(AudioCallback clb) = 0;
    virtual bool stop() = 0;
    virtual bool shutdown() = 0;
    virtual bool restart() = 0;
    virtual bool changeParameters(frame_t sampleRate, frame_t bufferSize, int numInputs, int numOutputs) = 0;

    const frame_t sampleRate() const { return _sampleRate; }
    const frame_t bufferSize() const { return _bufferSize; }
    
    const int inputCount() const { return _numInputs; }
    const int outputCount() const { return _numOutputs; }

    virtual const frame_t inputLatency(int port) = 0;
    virtual const frame_t outputLatency(int port) = 0;

    const bool isRunning() const { return _isRunning; }

    const frame_t framesPassed() const { return _framesPassed; }

    protected:
    AudioCallback _callback;
    frame_t _sampleRate;
    frame_t _bufferSize;
    int _numInputs;
    int _numOutputs;
    bool _isRunning;
    frame_t _framesPassed;
};

//Curiously Recurring Template Pattern
class AudioDriverFactory {
    public:
    using Creator = std::function<std::unique_ptr<AudioDriver>()>;

    static AudioDriverFactory& instance() {
        static AudioDriverFactory factory;
        return factory;
    }

    static void register_driver(const std::string& name, Creator create);

    static std::unique_ptr<AudioDriver> create(const std::string& name) {
        auto inst = instance();
        auto it = inst.registry.find(name);
        if(it != inst.registry.end()) {
            return it->second();
        } else {
            LOG_ERROR("Unknown driver, using Dummy Driver");
            auto it = inst.registry.find("Dummy Driver");
            if(it != inst.registry.end()) {
                return it->second();
            }
            LOG_FATAL("Error during loading driver, terminate");
        }

        return std::make_unique<dummy>(); // should never reach it, just to get rid of compile warning
    }

    private:
    std::unordered_map<std::string, Creator> registry;

    class dummy : public AudioDriver {
        public:
        dummy() {}
        ~dummy() {}
        
        bool init(frame_t sampleRate, frame_t bufferSize, int numInputs, int numOutputs) override {return true;}
        bool start(AudioCallback clb) override {return true;}
        bool stop() override {return true;}
        bool shutdown() override {return true;}
        bool restart() override {return true;}
        bool changeParameters(frame_t sampleRate, frame_t bufferSize, int numInputs, int numOutputs) override {return true;}
    
        const frame_t inputLatency(int port) override { return 0; }
        const frame_t outputLatency(int port) override { return 0; }
    };
};

template <typename T>
class DriverRegistrar {
    public:
    DriverRegistrar(const std::string& name) {
        AudioDriverFactory::instance().register_driver(name, [] {
            return std::make_unique<T>();
        });
    }
};

}