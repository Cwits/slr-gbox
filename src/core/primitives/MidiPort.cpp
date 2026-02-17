#include "core/primitives/MidiPort.h"

#include "slr_config.h"

#include "logger.h"
#include "core/primitives/MidiEvent.h"
#include "core/primitives/MidiPort.h"
#include "core/SettingsManager.h"
#include "core/ControlEngine.h"
#include "core/RtEngine.h"
#include "core/FlatEvents.h"
#include "core/MidiController.h"

#include <iostream>
#include <vector>
#include <algorithm>

#include <sys/eventfd.h>

namespace slr {

void event(MidiPort *port, const MidiEventType type, const int channel, const unsigned char *data);
void realTimeEvent(MidiPort *port, const MidiEventType type);
void sysexEvent(MidiPort *port, const unsigned char *data, const int &len);


ID _midiPortIdCounter = 0;

extern frame_t _input_constant_delay;
extern frame_t _sample_rate;
extern frame_t _block_size;


MidiPort::MidiPort() :
    _uniqueId(_midiPortIdCounter++),
    _justCreated(true)
{
    _rtLocalBuffer = std::make_unique<MidiBuffer>();
    // _rtLocalBuffer->reserve(MIDI_SPSCQUEUE_SIZE);
    _inQueue = std::make_unique<SPSCQueue<MidiEvent, MIDI_SPSCQUEUE_SIZE>>();
    _outQueue = std::make_unique<SPSCQueue<MidiEvent, MIDI_SPSCQUEUE_SIZE>>();
    _alternativeEventHandle = false;
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
        // LOG_INFO("Data! %s", &buf[0]);// %s", &buf[0]);
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
            if(!_alternativeEventHandle)
                realTimeEvent(this, static_cast<MidiEventType>(byte));
            else 
                _realTimeHandle(static_cast<MidiEventType>(byte));

            continue;
        }

        if(byte >= 0x80) {
            //command byte
            if(byte == 0xF7) {
                if(_currentCommand == MidiEventType::SysEx) {
                    //Sysex completed
                    if(!_alternativeEventHandle)
                        sysexEvent(this, &_rawData[0], _dataBytesReaded);
                    else 
                        _sysexHandle(&_rawData[0], _dataBytesReaded);

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
                if(!_alternativeEventHandle)
                    event(this, _currentCommand, 0, nullptr);
                else
                    _eventHandle(_currentCommand, 0, nullptr);
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
            if(!_alternativeEventHandle)
                event(this, _currentCommand, _channel, &_rawData[0]);
            else
                _eventHandle(_currentCommand, _channel, &_rawData[0]);

            _dataBytesReaded = 0;
        }
    }
}

void MidiPort::pushEvent(const MidiEventType type, const int channel, const int note, const int velocity) {
    
    std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = now - _controller->getAnchor(); //in seconds?
    frame_t lastSample = _controller->getLastSample();
    frame_t sample = diff.count() * _sample_rate;
    frame_t offset = lastSample + _input_constant_delay + sample;
    // frame_t offset = lastSample + sample;
    MidiEvent ev;
    ev.channel = channel;
    ev.type = type;
    ev.note = note;
    ev.velocity = velocity;
    ev.offset = offset;
    _inQueue->push(ev);
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

void MidiPort::setAlternativeHandles(
        std::function<void(const MidiEventType)> realTimeHandle,
        std::function<void(const unsigned char *, const int &)> sysexHandle,
        std::function<void(const MidiEventType, const int, const unsigned char *)> eventHandle
    ) 
{
    if(!_alternativeEventHandle) {
        _realTimeHandle = std::move(realTimeHandle);
        _sysexHandle = std::move(sysexHandle);
        _eventHandle = std::move(eventHandle);
        _alternativeEventHandle = true;
    } else {
        LOG_ERROR("Alternative handles for port %s already set", _ownerSubdev->_path.c_str());
    }
}


void event(MidiPort *port, const MidiEventType type, const int channel, const unsigned char *data) {
    LOG_INFO("Event");
    switch(type) {
        case(MidiEventType::NoteOff): {
            int note = static_cast<int>(data[0]);
            int velocity = static_cast<int>(data[1]);
            port->pushEvent(type, channel, note, velocity);
#if (MIDI_TRACE == 1)
            LOG_INFO("Note Off channel %d note %d vel %d",
                        channel, note, velocity);
#endif
        } break;
        case(MidiEventType::NoteOn): {
            int note = static_cast<int>(data[0]);
            int velocity = static_cast<int>(data[1]);
            if(velocity > 0) {
                port->pushEvent(type, channel, note, velocity);
#if (MIDI_TRACE == 1)
                LOG_INFO("Note On channel %d note %d vel %d",
                        channel, note, velocity);
#endif
            } else {
                port->pushEvent(MidiEventType::NoteOff, channel, note, velocity);
#if (MIDI_TRACE == 1)
                LOG_INFO("Note On(Off) channel %d note %d vel %d",
                        channel, note, velocity);
#endif      
            }
        } break;
        case(MidiEventType::Aftertouch): { 
            int note = static_cast<int>(data[0]);
            int pressure = static_cast<int>(data[1]);
            port->pushEvent(type, channel, note, pressure);
#if (MIDI_TRACE == 1)
            LOG_INFO("Aftertouch channel %d note %d pressure %d",
                        channel, note, pressure);
#endif      
        } break;
        case(MidiEventType::CC): { 
            int cc = static_cast<int>(data[0]);
            int value = static_cast<int>(data[1]);
            port->pushEvent(type, channel, cc, value);
#if (MIDI_TRACE == 1)
            LOG_INFO("CC channel %d cc %d value %d",
                        channel, cc, value);
#endif                  
        } break;
        case(MidiEventType::ProgramChange): { 
            int program = static_cast<int>(data[0]);
#if (MIDI_TRACE == 1)
            LOG_INFO("Program Change channel %d program %d",
                        channel, program);
#endif
        } break;
        case(MidiEventType::ChannelPressure): { 
            int pressure = static_cast<int>(data[0]);
            port->pushEvent(type, channel, pressure, 0);
#if (MIDI_TRACE == 1)
            LOG_INFO("Channel Pressure channel %d pressure %d",
                        channel, pressure);
#endif                  
        } break;
        case(MidiEventType::PitchBend): {
            int lsb = static_cast<int>(data[0]);
            int msb = static_cast<int>(data[1]);
            int pitch = lsb | (msb << 7);
            port->pushEvent(type, channel, pitch, 0);
#if (MIDI_TRACE == 1)
            LOG_INFO("Pitch Bend channel %d pitch %d",
                        channel, pitch);
#endif                  
        } break;
        case(MidiEventType::SongPosition): {
            int lsb = static_cast<int>(data[0]);
            int msb = static_cast<int>(data[1]);
#if (MIDI_TRACE == 1)
            LOG_INFO("Song Position lsb %x msb %x",
                        lsb, msb);
#endif
        } break;
        case(MidiEventType::SongSelect): {
            int song = static_cast<int>(data[0]);
#if (MIDI_TRACE == 1)
            LOG_INFO("Song Select song %d", song);
#endif
        } break;
        case(MidiEventType::TimeCodeQuarterFrame): {
            int dta = static_cast<int>(data[0]);
#if (MIDI_TRACE == 1)
            LOG_INFO("Time Code Quater Frame data %d", dta);
#endif
        } break;
        case(MidiEventType::TuneRequest): {
#if (MIDI_TRACE == 1)
            LOG_INFO("Tune Request");
#endif
        }
    }
}

void realTimeEvent(MidiPort *port, const MidiEventType type) {
    switch(type) {
        case(MidiEventType::Clock): {
#if (MIDI_TRACE == 1)
            LOG_INFO("Clock"); 
#endif
        } break;
        case(MidiEventType::Start): {
#if (MIDI_TRACE == 1)
            LOG_INFO("Start"); 
#endif
        } break;
        case(MidiEventType::Continue): {
#if (MIDI_TRACE == 1)
            LOG_INFO("Continue"); 
#endif
        } break;
        case(MidiEventType::Stop): {
#if (MIDI_TRACE == 1)
            LOG_INFO("Stop"); 
#endif
        } break;
        case(MidiEventType::ActiveSensing): {
#if (MIDI_TRACE == 1)
            LOG_INFO("Active Sensing"); 
#endif
        } break;
        case(MidiEventType::SystemReset): {
#if (MIDI_TRACE == 1)
            LOG_INFO("System Reset");
#endif
            port->_dataBytesReaded = 0;
            port->_currentCommand = MidiEventType::InvalidType;
            port->_channel = 0;
            port->_expectedLength = 0;
        } break;
    }
}

void sysexEvent(MidiPort *port, const unsigned char *data, const int &len) {
#if (MIDI_TRACE == 1)
    LOG_INFO("SysEx event length: %d", len);
    // pr(std::hex);
    // pr("0xf0 ");
    // for(int i=0; i<len; ++i) {
        // pr((int)data[i] << " ");
    // }
    // pr("f7")
    // prl(std::dec);
#endif
}

}