// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/MidiController.h"


// #include <rtmidi/RtMidi.h>
#include "logger.h"
#include "core/primitives/MidiEvent.h"
#include "core/SettingsManager.h"

#include <iostream>
#include <vector>
#include <algorithm>

// #include <unistd.h> //close(fd)
// #include <fcntl.h> //open() O_RDONLY
#include <sys/eventfd.h>

namespace slr {

void event(MidiPort *port, const MidiEventType type, const int channel, const unsigned char *data);
void realTimeEvent(MidiPort *port, const MidiEventType type);
void sysexEvent(const unsigned char *data, const int &len);
int expectedLength(const MidiEventType &type);

ID _midiPortIdCounter = 1;

frame_t _input_constant_delay = 0;
frame_t _sample_rate = 0;

MidiController::MidiController() {
    _input_constant_delay = SettingsManager::getBlockSize();
    _sample_rate = SettingsManager::getSampleRate();
}

MidiController::~MidiController() {
    for(std::unique_ptr<MidiPort> &port : _activePorts) {
        closeDevice(port.get());
        // delete port;
    }
}

void MidiController::checkDevices() {
    std::vector<MidiDevice> list = discoverMidiDevices();

    std::lock_guard<std::mutex> l(_mutex);

    for(MidiDevice &exist : _deviceList) {
        exist._check = false;
    }

    for(MidiDevice &devFound : list) {
        std::vector<MidiDevice>::iterator it;
        it = std::find_if(
            _deviceList.begin(), 
            _deviceList.end(),
            [&](const MidiDevice &exist) {
                return (exist._card == devFound._card && 
                        exist._name == devFound._name);
            }
        );

        if(it != _deviceList.end()) {
            if(!it->_online) {
                //device got back
                it->_online = true;
                LOG_INFO("Device connected back %d", it->_card);
            }
            it->_check = true;
        } else {
            //new device
            devFound._check = true;
            devFound._online = true;
            _deviceList.push_back(devFound);
            LOG_INFO("New device %d", devFound._card);
        }
    }

    for(MidiDevice &dev : _deviceList) {
        if(!dev._check) {
            if(dev._online) { //already there
                dev._online = false;
                LOG_INFO("Device disconnected %i", dev._card);
            }       
        }
    }
}

std::vector<MidiDevice> MidiController::devList() {
    std::vector<MidiDevice> ret;
    
    {
        std::lock_guard<std::mutex> lock(_mutex);
        ret = _deviceList;
    }

    return ret;
}

void MidiController::openDevice(MidiPort *port, bool input, bool output) {
    if(port->_isOpened &&
        port->_inputOpened == input &&
        port->_outputOpened == output) 
    {
        //nothing to change
        return;
    }

    closeDevice(port); //i don't like this

    if(!input && !output) return; //i don't like this

    //open
    int ret = snd_rawmidi_open(
        input  ? &port->_inputHandle  : NULL,
        output ? &port->_outputHandle : NULL,
        port->_path.c_str(),
        0
    );

    if(ret < 0) {
        // ошибка — оставить порт закрытым
        port->_isOpened      = false;
        port->_inputOpened   = false;
        port->_outputOpened  = false;
        port->_inputHandle   = nullptr;
        port->_outputHandle  = nullptr;
        return;
    }

    port->_isOpened        = true;
    port->_inputOpened   = input;
    port->_outputOpened  = output;
    port->_anchorTimepoint = _midiAnchorTimepoint;

    //start reading thread if necessary
    if(input)
        port->_readingThread = std::thread(&MidiPort::inputHandle, port);

    if(output)
        port->_writingThread = std::thread(&MidiPort::writeHandle, port);
}

void MidiController::closeDevice(MidiPort *port) {
    if(port->_inputHandle) {
        port->_run = false;
        port->_readingThread.join();

        snd_rawmidi_close(port->_inputHandle);
    }

    if(port->_outputHandle) {
        port->_runOutput = false;
        port->_outputOpened = false;
        port->_writingThread.join();

        snd_rawmidi_close(port->_outputHandle);
    }

    port->_isOpened = false;
    port->_inputOpened = false;
    port->_outputOpened = false;
    port->_inputHandle = nullptr;
    port->_outputHandle = nullptr;

    // if(port->_inQueue) port->_inQueue.reset();
    // if(port->_outQueue) port->_outQueue.reset();
}


std::vector<MidiDevice> MidiController::discoverMidiDevices() {
    std::vector<MidiDevice> ret;

    int status;
    int card = -1;

    while((status = snd_card_next(&card)) >= 0 && card >= 0) {
        snd_ctl_t *ctl;
        std::string path;
        int device = -1;

        path.clear();
        path.append("hw:");
        path.append(std::to_string(card));
        
        if((status = snd_ctl_open(&ctl, path.c_str(), 0)) < 0) {
            //err opening card
            continue;
        }

        MidiDevice dev;
        dev._card = card;
        bool appendDev = false;
        
        while((status = snd_ctl_rawmidi_next_device(ctl, &device)) >= 0 &&
                device >= 0) 
        {
            //explore subdevs
            snd_rawmidi_info_t *info;
            const char *name;
            int subsMax, subsIn, subsOut;

            snd_rawmidi_info_alloca(&info);
            snd_rawmidi_info_set_device(info, device);
            
            snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_INPUT);
            snd_ctl_rawmidi_info(ctl, info);
            subsIn = snd_rawmidi_info_get_subdevices_count(info);
            name = snd_rawmidi_info_get_name(info);

            snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_OUTPUT);
            snd_ctl_rawmidi_info(ctl, info);
            subsOut = snd_rawmidi_info_get_subdevices_count(info);

            subsMax = subsIn > subsOut ? subsIn : subsOut;
            if(subsMax != 0) {
                appendDev = true;
            }

            dev._name = std::string(name);

            int sub = 0;
            bool hasInput = false;
            bool hasOutput = false;
            const char *subInName = nullptr;
            const char *subOutName = nullptr;
            while(sub < subsMax) {
                hasInput = is_input(ctl, card, device, sub);
                hasOutput = is_output(ctl, card, device, sub);
                
                if(hasInput) {
                    snd_rawmidi_info_set_subdevice(info, sub);
                    snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_INPUT);
                    snd_ctl_rawmidi_info(ctl, info);
                    subInName = snd_rawmidi_info_get_subdevice_name(info);
                }

                if(hasOutput) {
                    snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_OUTPUT);
                    snd_ctl_rawmidi_info(ctl, info);
                    subOutName = snd_rawmidi_info_get_subdevice_name(info);
                }

                MidiSubdevice subdev;
                subdev._hasInput = hasInput;
                subdev._hasOutput = hasOutput;
                std::string subpath = "hw:";
                subpath.append(std::to_string(card)).append(",");
                subpath.append(std::to_string(device)).append(",");
                subpath.append(std::to_string(sub));
                subdev._path = subpath;
                if(hasInput) subdev._inputName = std::string(subInName);
                if(hasOutput) subdev._outputName = std::string(subOutName);

                dev._ports.push_back(subdev);
                // printf("%c%c  hw:%d,%d,%d  %s\n",
                //                 hasInput ? 'I' : ' ', hasOutput ? 'O' : ' ',
                //                 card, device, sub, subName);
                sub++;
            }
        }

        snd_ctl_close(ctl);

        if(appendDev)
            ret.push_back(dev);
    }

    return ret;
}

bool MidiController::is_input(snd_ctl_t *ctl, int card, int device, int sub) {
    snd_rawmidi_info_t *info;
    int status;

    snd_rawmidi_info_alloca(&info);
    snd_rawmidi_info_set_device(info, device);
    snd_rawmidi_info_set_subdevice(info, sub);
    snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_INPUT);
    
    if ((status = snd_ctl_rawmidi_info(ctl, info)) < 0 && status != -ENXIO) {
        return false;
    } else if (status == 0) {
        return true;
    }

    return false;
}

bool MidiController::is_output(snd_ctl_t *ctl, int card, int device, int sub) {
    snd_rawmidi_info_t *info;
    int status;

    snd_rawmidi_info_alloca(&info);
    snd_rawmidi_info_set_device(info, device);
    snd_rawmidi_info_set_subdevice(info, sub);
    snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_OUTPUT);
    
    if ((status = snd_ctl_rawmidi_info(ctl, info)) < 0 && status != -ENXIO) {
        return false;
    } else if (status == 0) {
        return true;
    }

    return false;
}

MidiPort::MidiPort() :
    _uniqueId(_midiPortIdCounter++),
    _justCreated(true)
{
    _rtLocalBuffer = std::make_unique<std::vector<MidiEvent>>();
    _rtLocalBuffer->reserve(MIDI_SPSCQUEUE_SIZE);
    _inQueue = std::make_unique<SPSCQueue<MidiEvent, MIDI_SPSCQUEUE_SIZE>>();
    _outQueue = std::make_unique<SPSCQueue<MidiEvent, MIDI_SPSCQUEUE_SIZE>>();
}

MidiPort::~MidiPort() {
    //we are closing at ~MidiController
    // _run = false;
    // _readingThread.join();

    // if(_inputHandle) snd_rawmidi_close(_inputHandle);
    // if(_outputHandle) snd_rawmidi_close(_outputHandle);
}

constexpr int MIDI_RAW_INPUT_SIZE = 128;

/* inspired and Big Thanks to LMMS*/
void MidiPort::inputHandle(MidiPort *port) {
    unsigned char buf[MIDI_RAW_INPUT_SIZE];
  
    // std::atomic<bool> & run = port->_run;
    port->_run = true;

    snd_rawmidi_t * in = port->_inputHandle;
	snd_rawmidi_nonblock(in, 1);
	int npfds = snd_rawmidi_poll_descriptors_count(in);
	pollfd * pfds = new pollfd[npfds];
	snd_rawmidi_poll_descriptors(in, pfds, npfds);

    while(port->_run) {
        std::this_thread::sleep_for(std::chrono::microseconds(5));

        int err = poll(pfds, npfds, 10000);

        if(err < 0 && errno == EINTR) {
            // printf( "MidiAlsaRaw::run(): Got EINTR while "
			// 	"polling. Will stop polling MIDI-events from "
			// 	"MIDI-port.\n" );
            break;
        }
        if(err < 0) {
            // printf( "poll failed: %s\nWill stop polling "
			// 	"MIDI-events from MIDI-port.\n",
			// 				strerror( errno ) );
            break;
        }

        if(err == 0) continue;

        unsigned short revents;
        err = snd_rawmidi_poll_descriptors_revents(in, pfds, npfds, &revents);

        if(err < 0) {
            // printf( "cannot get poll events: %s\nWill stop polling "
			// 	"MIDI-events from MIDI-port.\n",
			// 				snd_strerror( errno ) );
            break;
        }

        if(revents & (POLLERR | POLLHUP)) {
            // printf( "POLLERR or POLLHUP\n" );
            break;
        }
 
        if(!(revents & POLLIN)) continue;

        err = snd_rawmidi_read(in, buf, sizeof(buf));
        if(err == -EAGAIN) continue;

        if(err < 0) {
            // printf( "cannot read from port \"%s\": %s\nWill stop "
			// 	"polling MIDI-events from MIDI-port.\n",
			// 	/*port_name*/"default", snd_strerror( err ) );
			break;
        }

        if(err == 0) continue;

        // buf[127] = '\0';
        // LOG_INFO("Data!");// %s", &buf[0]);
        // for(int i=0; i<err; ++i) {
        port->parseInput(&buf[0], err);
        // }
    }

    delete [] pfds;
}

void MidiPort::parseInput(unsigned char *raw, const int &size) {
    int bytenum = 0;

    while(true) {
        if(bytenum >= size) break;
        int byte = raw[bytenum++];
        
        if(byte >= 0xF8) {
            //real-time msg
            //0xFD and 0xF9 just ignored
            realTimeEvent(this, static_cast<MidiEventType>(byte));
            continue;
        }

        if(byte >= 0x80) {
            //command byte
            if(byte == 0xF7) {
                if(_currentCommand == MidiEventType::SysEx) {
                    //Sysex completed
                    sysexEvent(&_rawData[0], _dataBytesReaded);
                } else {
                    // prl("error, unexpected 0xF7 byte");
                }

                _currentCommand = MidiEventType::InvalidType;
                _dataBytesReaded = 0;
                continue;
            }

            if(byte == 0xF4 || byte == 0xF5) {
                // prl("error, undefined status byte, previous status " << std::hex <<
                //     static_cast<int>(_currentCommand) << " readed data " << std::dec <<
                //     _dataBytesReaded);

                _currentCommand = MidiEventType::InvalidType;
                _dataBytesReaded = 0;
            }

            if(_dataBytesReaded != 0) {
                // prl("new command during unfinished data reading, previous " << std::hex <<
                //     static_cast<int>(_currentCommand) << " readed data " << std::dec <<
                //     _dataBytesReaded);
                _dataBytesReaded = 0;
            }

            _currentCommand = (byte > 0xF0) ? 
                                    static_cast<MidiEventType>(byte) : 
                                    static_cast<MidiEventType>(byte & 0xF0);
            _channel = (byte < 0xF0) ? (byte & 0x0F)+1 : 0;
            _expectedLength = expectedLength(_currentCommand);

            if(_currentCommand == MidiEventType::TuneRequest) {
                // prl("tune request");
                event(this, _currentCommand, 0, nullptr);
            }

            continue;
        }

        //data byte
        if(_currentCommand == MidiEventType::InvalidType) {
            // LOG_ERROR("garbage data?");
            continue;
        }

        if(_currentCommand == MidiEventType::SysEx) {
            if(_dataBytesReaded < 128) {
                _rawData[_dataBytesReaded++] = byte;
            } else {
                // LOG_ERROR("sysex overflow");
            }

            continue;
        }

        if(_dataBytesReaded < _expectedLength) {
            _rawData[_dataBytesReaded++] = byte;
        }

        if(_dataBytesReaded == _expectedLength) {
            event(this, _currentCommand, _channel, &_rawData[0]);
            _dataBytesReaded = 0;
        }
    }
}

void MidiPort::pushEvent(const MidiEventType type, const int channel, const int note, const int velocity) {
    
    std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
    frame_t millis = std::chrono::duration_cast<std::chrono::milliseconds>(now - _anchorTimepoint).count();
    // frame_t sample = offset * (1.0f/_sample_rate);
    frame_t sample = millis * (1.0f/_sample_rate);

    MidiEvent ev;
    ev.channel = channel;
    ev.type = type;
    ev.note = note;
    ev.velocity = velocity;
    ev.offset = sample;
    _inQueue->push(ev);
}

void event(MidiPort *port, const MidiEventType type, const int channel, const unsigned char *data) {
    switch(type) {
        case(MidiEventType::NoteOff): {
            int note = static_cast<int>(data[0]);
            int velocity = static_cast<int>(data[1]);
            LOG_INFO("Note Off channel %d note %d vel %d",
                        channel, note, velocity);
            port->pushEvent(type, channel, note, velocity);
        } break;
        case(MidiEventType::NoteOn): {
            int note = static_cast<int>(data[0]);
            int velocity = static_cast<int>(data[1]);
            if(velocity > 0) {
                LOG_INFO("Note On channel %d note %d vel %d",
                        channel, note, velocity);
                port->pushEvent(type, channel, note, velocity);
            } else {
                LOG_INFO("Note On(Off) channel %d note %d vel %d",
                        channel, note, velocity);
                port->pushEvent(MidiEventType::NoteOff, channel, note, velocity);
            }
        } break;
        case(MidiEventType::Aftertouch): { 
            int note = static_cast<int>(data[0]);
            int pressure = static_cast<int>(data[1]);
            LOG_INFO("Aftertouch channel %d note %d pressure %d",
                        channel, note, pressure);
            port->pushEvent(type, channel, note, pressure);
        } break;
        case(MidiEventType::CC): { 
            int cc = static_cast<int>(data[0]);
            int value = static_cast<int>(data[1]);
            LOG_INFO("CC channel %d cc %d value %d",
                        channel, cc, value);
                        
            port->pushEvent(type, channel, cc, value);
        } break;
        case(MidiEventType::ProgramChange): { 
            int program = static_cast<int>(data[0]);
            LOG_INFO("Program Change channel %d program %d",
                        channel, program);
        } break;
        case(MidiEventType::ChannelPressure): { 
            int pressure = static_cast<int>(data[0]);
            LOG_INFO("Channel Pressure channel %d pressure %d",
                        channel, pressure);
                        
            port->pushEvent(type, channel, pressure, 0);
        } break;
        case(MidiEventType::PitchBend): {
            int lsb = static_cast<int>(data[0]);
            int msb = static_cast<int>(data[1]);
            int pitch = lsb | (msb << 7);
            LOG_INFO("Pitch Bend channel %d pitch %d",
                        channel, pitch);
                        
            port->pushEvent(type, channel, pitch, 0);
        } break;
        case(MidiEventType::SongPosition): {
            int lsb = static_cast<int>(data[0]);
            int msb = static_cast<int>(data[1]);
            LOG_INFO("Song Position lsb %x msb %x",
                        lsb, msb);
        } break;
        case(MidiEventType::SongSelect): {
            int song = static_cast<int>(data[0]);
            LOG_INFO("Song Select song %d", song);
        } break;
        case(MidiEventType::TimeCodeQuarterFrame): {
            int dta = static_cast<int>(data[0]);
            LOG_INFO("Time Code Quater Frame data %d", dta);
        } break;
        case(MidiEventType::TuneRequest): {
            LOG_INFO("Tune Request");
        }
    }
}

void realTimeEvent(MidiPort *port, const MidiEventType type) {
    switch(type) {
        case(MidiEventType::Clock): LOG_INFO("Clock"); break;
        case(MidiEventType::Start): LOG_INFO("Start"); break;
        case(MidiEventType::Continue): LOG_INFO("Continue"); break;
        case(MidiEventType::Stop): LOG_INFO("Stop"); break;
        case(MidiEventType::ActiveSensing): /*LOG_INFO("Active Sensing")*/; break;
        case(MidiEventType::SystemReset): {
            LOG_INFO("System Reset"); break;
            port->_dataBytesReaded = 0;
            port->_currentCommand = MidiEventType::InvalidType;
            port->_channel = 0;
            port->_expectedLength = 0;
        }
    }
}

void sysexEvent(const unsigned char *data, const int &len) {
    // pr("SysEx event length " << len << " data: ");
    // pr(std::hex);
    // pr("0xf0 ");
    LOG_INFO("SysEx event length: %d", len);
    // for(int i=0; i<len; ++i) {
        // pr((int)data[i] << " ");
    // }
    // pr("f7")
    // prl(std::dec);
}

int expectedLength(const MidiEventType &type) {
    switch(type) {
        case(MidiEventType::NoteOff):
        case(MidiEventType::NoteOn):
        case(MidiEventType::Aftertouch):
        case(MidiEventType::CC):
        case(MidiEventType::PitchBend):
        case(MidiEventType::SongPosition): return 2; break;
        case(MidiEventType::ProgramChange):
        case(MidiEventType::ChannelPressure):
        case(MidiEventType::TimeCodeQuarterFrame):
        case(MidiEventType::SongSelect): return 1; break;
        case(MidiEventType::SysEx): return -1; break;
        default: return 0; break;
    }
}

/* some LLM stuff, need testing. don't want to dive deep into alsa right now */
void MidiPort::writeHandle(MidiPort *port) {
    snd_rawmidi_t *out = port->_outputHandle;
    auto *queue = port->outQueue();

    snd_rawmidi_nonblock(out, 1);

    int alsa_fds = snd_rawmidi_poll_descriptors_count(out);
    pollfd *alsa_pfds = new pollfd[alsa_fds];
    snd_rawmidi_poll_descriptors(out, alsa_pfds, alsa_fds);

    int efd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);

    pollfd idle_pfd{};
    idle_pfd.fd = efd;
    idle_pfd.events = POLLIN;

    pollfd *write_pfds = new pollfd[alsa_fds + 1];
    for (int i = 0; i < alsa_fds; ++i)
        write_pfds[i] = alsa_pfds[i];

    write_pfds[alsa_fds].fd = efd;
    write_pfds[alsa_fds].events = POLLIN;

    uint8_t buffer[256];
    size_t buf_used = 0;

    port->_efd = efd;
    port->_outputOpened = true;
    port->_runOutput = true;

    while(port->_runOutput) {

        if(buf_used == 0 && queue->empty()) {
            idle_pfd.revents = 0;
            poll(&idle_pfd, 1, -1);

            if(idle_pfd.revents & POLLIN) {
                uint64_t v;
                read(efd, &v, sizeof(v));
            }
            continue;
        }

        while(buf_used + 3 <= sizeof(buffer)) {
            MidiEvent ev;
            if(!queue->pop(ev))
                break;

            buffer[buf_used + 0] = static_cast<uint8_t>(ev.type) | (ev.channel & 0x0F);
            buffer[buf_used + 1] = static_cast<uint8_t>(ev.note & 0x7F);
            buffer[buf_used + 2] = static_cast<uint8_t>(ev.velocity & 0x7F);
            buf_used += 3;
        }

        if(buf_used == 0)
            continue;

        ssize_t r = snd_rawmidi_write(out, buffer, buf_used);

        if(r > 0) {
            memmove(buffer, buffer + r, buf_used - r);
            buf_used -= r;
            continue;
        }

        if(r == -EAGAIN) {
            for(int i=0; i<alsa_fds+1; ++i)
                write_pfds[i].revents = 0;

            poll(write_pfds, alsa_fds + 1, -1);

            if(write_pfds[alsa_fds].revents & POLLIN) {
                uint64_t v;
                read(efd, &v, sizeof(v));
            }
            continue;
        }

        break;
    }

    close(efd);
    delete[] alsa_pfds;
    delete[] write_pfds;
}


}