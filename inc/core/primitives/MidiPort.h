// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/primitives/SPSCQueue.h"
#include "core/primitives/MidiEvent.h"
#include "core/primitives/MidiBuffer.h"
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
#include <functional>

namespace slr {

class RtMidiOutput;
class MidiController;
class MidiDevice;
class MidiSubdevice;
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

    //from device to input queue
    void pushEvent(const MidiEventType type, const int channel, const int note, const int velocity);

    //from output queue to device
    void sendMidi(const MidiEvent &ev);

    MidiBuffer * rtLocalBuffer() { return _rtLocalBuffer.get(); }
    SPSCQueue<MidiEvent, MIDI_SPSCQUEUE_SIZE> * inQueue() { return _inQueue.get();}
    SPSCQueue<MidiEvent, MIDI_SPSCQUEUE_SIZE> * outQueue() { return _outQueue.get(); }

    void setAlternativeHandles(
        std::function<void(const MidiEventType)> realTimeHandle,
        std::function<void(const unsigned char *, const int &)> sysexHandle,
        std::function<void(const MidiEventType, const int, const unsigned char *)> eventHandle
    );

    private:
    const ID _uniqueId;

    snd_rawmidi_t* _inputHandle = nullptr;
    snd_rawmidi_t* _outputHandle = nullptr;

    std::unique_ptr<MidiBuffer> _rtLocalBuffer;
    std::unique_ptr<SPSCQueue<MidiEvent, MIDI_SPSCQUEUE_SIZE>> _inQueue;
    std::unique_ptr<SPSCQueue<MidiEvent, MIDI_SPSCQUEUE_SIZE>> _outQueue;

    bool _isOpened;
    bool _inputOpened;
    std::atomic<bool> _outputOpened;
    bool _justCreated;

    bool _alternativeEventHandle;
    std::function<void(const MidiEventType)> _realTimeHandle;
    std::function<void(const unsigned char *, const int &)> _sysexHandle;
    std::function<void(const MidiEventType, const int, const unsigned char *)> _eventHandle;

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

}