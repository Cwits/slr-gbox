// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/primitives/SPSCQueue.h"
#include "core/primitives/MidiEvent.h"
#include "core/primitives/MidiBuffer.h"
#include "core/primitives/MidiPort.h"
#include "defines.h"

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <functional>

namespace slr {

struct MidiController;
struct MidiSubdevice;
class MidiPort;

enum class DevicePort { INPUT, OUTPUT };

struct MidiDevice {
    // MidiDevice(const MidiDevice &other) = delete;
    std::string _name;
    std::vector<MidiSubdevice> _ports;
    int _card;
    // bool _presented;
    bool _check;

    bool _online; //use this instead of check\presented??
};

struct MidiSubdevice {
    //actual thing here
    bool _hasInput;
    bool _hasOutput;
    std::string _path;
    std::string _inputName;
    std::string _outputName;    
};

//for use in RT only
struct RtMidiQueue {
    ID id;
    SPSCQueue<MidiEvent, MIDI_SPSCQUEUE_SIZE> *queue;
};

struct RtMidiOutput {
    RtMidiOutput(MidiPort *port, ID id) : id(id), port(port) {}
    const ID id;
    void sendEvent(const MidiEvent &ev) {
        if(!port->outputOpened()) return;

        bool wasEmpty = port->outQueue()->empty();
        port->outQueue()->push(ev);
        if(wasEmpty) {
            uint64_t one = 1;
            write(port->_efd, &one, sizeof(one));
        }
    }

    private:
    MidiPort *port;
};

struct RtMidiBuffer {
    ID id;
    MidiBuffer * buffer;
};

class ControlContext;
namespace Events {
    class VirtualMidiKbdAction;
}

struct MidiController {
    MidiController();
    ~MidiController();

    void checkDevices();
    std::vector<MidiDevice> devList();
    std::vector<std::unique_ptr<MidiPort>> & activePorts() { 
        return _activePorts;
    }
    const std::vector<std::unique_ptr<MidiPort>> & activePorts() const { 
        return _activePorts;
    }

    void addNewPort(std::unique_ptr<MidiPort> port) {
        port->_controller = this;
        _activePorts.push_back(std::move(port));
    }

    void openDevice(MidiPort *port, bool input, bool output);
    void closeDevice(MidiPort *port);

    void setAnchor(frame_t framesPassed) {
        _midiAnchorTimepoint.store(std::chrono::steady_clock::now(), std::memory_order_relaxed);
        _lastSample.store(framesPassed, std::memory_order_relaxed);
    }
    std::chrono::time_point<std::chrono::steady_clock> getAnchor() const { return _midiAnchorTimepoint.load(std::memory_order_acquire); }
    frame_t getLastSample() const { return _lastSample.load(std::memory_order_acquire); }

    private:
    std::mutex _mutex;
    std::vector<MidiDevice> _deviceList;
    std::vector<std::unique_ptr<MidiPort>> _activePorts;

    std::vector<MidiDevice> discoverMidiDevices();
    bool is_input(snd_ctl_t *ctl, int card, int device, int sub);
    bool is_output(snd_ctl_t *ctl, int card, int device, int sub);

    std::atomic<std::chrono::time_point<std::chrono::steady_clock>> _midiAnchorTimepoint;
    std::atomic<frame_t> _lastSample;

    friend void handleEvent(const ControlContext &ctx, const Events::VirtualMidiKbdAction &e);
    void addVirtualKbdEvent(const MidiEvent ev);
};

}