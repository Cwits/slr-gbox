// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/primitives/SPSCQueue.h"
#include "core/primitives/MidiEvent.h"
#include "defines.h"

#include <alsa/asoundlib.h>

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

namespace slr {

struct MidiController;
struct MidiSubdevice;

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

//MidiPorts should be added, but never deleted during current session
//even if disabled by user or disconnected
//(only on application close)
struct MidiPort {
    MidiPort();
    ~MidiPort();
    MidiDevice * _ownerDev;
    MidiSubdevice * _ownerSubdev;
    std::string _path;
    
    const ID id() const { return _uniqueId; }
    bool isOpened() const { return _isOpened; }
    bool inputOpened() const { return _inputOpened; }
    bool outputOpened() const { return _outputOpened; }

    bool justCreated() const { return _justCreated; }
    void portsAddedToRt() { _justCreated = false; }

    std::vector<MidiEvent> * rtLocalBuffer() { return _rtLocalBuffer.get(); }
    SPSCQueue<MidiEvent, MIDI_SPSCQUEUE_SIZE> * inQueue() { return _inQueue.get();}
    SPSCQueue<MidiEvent, MIDI_SPSCQUEUE_SIZE> * outQueue() { return _outQueue.get(); }

    private:
    const ID _uniqueId;

    snd_rawmidi_t* _inputHandle = nullptr;
    snd_rawmidi_t* _outputHandle = nullptr;

    std::unique_ptr<std::vector<MidiEvent>> _rtLocalBuffer;
    std::unique_ptr<SPSCQueue<MidiEvent, MIDI_SPSCQUEUE_SIZE>> _inQueue;
    std::unique_ptr<SPSCQueue<MidiEvent, MIDI_SPSCQUEUE_SIZE>> _outQueue;

    bool _isOpened;
    bool _inputOpened;
    std::atomic<bool> _outputOpened;
    bool _justCreated;

    // std::chrono::time_point<std::chrono::steady_clock> _anchorTimepoint;
    MidiController * _controller; 

    std::atomic<bool> _run;
    std::thread _readingThread;

    static void inputHandle(MidiPort *port);
    void sendByte(unsigned char c);

    //parser stuff
    MidiEventType _currentCommand;
    int _expectedLength;
    int _channel;
    int _dataBytesReaded;
    unsigned char _rawData[128];
    void parseInput(unsigned char *raw, const int &size);
    void pushEvent(const MidiEventType type, const int channel, const int note, const int velocity);

    friend class MidiController;
    friend void event(MidiPort *port, const MidiEventType type, const int channel, const unsigned char *data);
    friend void realTimeEvent(MidiPort *port, const MidiEventType type);

    //writing stuff
    std::atomic<bool> _runOutput;
    std::thread _writingThread;
    int _efd;
    static void writeHandle(MidiPort *port);
    friend class RtMidiOutput;
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

struct MidiController {
    MidiController();
    ~MidiController();

    void checkDevices();
    std::vector<MidiDevice> devList();
    std::vector<std::unique_ptr<MidiPort>> & activePorts() { 
        return _activePorts;
    }
    const std::vector<std::unique_ptr<MidiPort>> & activePortsConst() const { 
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

};

}