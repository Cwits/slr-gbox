// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/MidiController.h"

#include "slr_config.h"

#include "logger.h"
#include "core/primitives/MidiEvent.h"
#include "core/primitives/MidiPort.h"
#include "core/SettingsManager.h"
#include "core/ControlEngine.h"
#include "core/RtEngine.h"
#include "core/FlatEvents.h"

#if (USE_PUSH == 1)
// #include "push/PushCore.h"
#include "ui/pushThread.h"
// #include "ui/push/MainWindow.h" //does not belong here...
#endif

#include <alsa/asoundlib.h>
#include <iostream>
#include <vector>
#include <algorithm>

#include <sys/eventfd.h>

namespace slr {

frame_t _input_constant_delay = 0;
frame_t _sample_rate = 0;
frame_t _block_size = 0;

MidiDevice _virtualDev;
MidiSubdevice _virtualSub;
std::unique_ptr<MidiPort> _virtualPort;


#if (USE_PUSH == 1)
#include "ui/pushThread.h"
// std::unique_ptr<PushLib::PushCore> _pushDev;
// std::unique_ptr<PushUI::MainWindow> _pushMainWindow;
#endif

MidiController::MidiController() {
    _input_constant_delay = SettingsManager::getBlockSize();
    _sample_rate = SettingsManager::getSampleRate();
    _block_size = SettingsManager::getBlockSize();
    
    //init virtual midi
    _virtualDev._name = "Virtual Midi";
    _virtualDev._card = 0;
    _virtualDev._check = true;
    _virtualDev._online = true;
    _virtualSub._hasInput = true;
    _virtualSub._hasOutput = false;
    _virtualSub._path = "";
    _virtualSub._inputName = "Virtual Midi Input";
    _virtualSub._outputName = "";
    _virtualDev._ports.push_back(_virtualSub);
    _virtualPort = std::make_unique<MidiPort>();
    _virtualPort->_ownerDev = &_virtualDev;
    _virtualPort->_ownerSubdev = &_virtualDev._ports.at(0);
    _virtualPort->_path = "";
    _virtualPort->_isOpened = true;
    _virtualPort->_inputOpened = true;
    _virtualPort->_controller = this;

    MidiPort * port = _virtualPort.get();
    RtEngine *engine = ControlEngine::rtEngine();
    _activePorts.push_back(std::move(_virtualPort));
    {
        const std::vector<RtMidiBuffer> *midiLocalBuffers = engine->midiLocalBuffers();
        const std::vector<RtMidiQueue> *midiInMap = engine->midiInMap();
        const std::vector<RtMidiOutput> *midiOutMap = engine->midiOutMap();

        std::vector<RtMidiBuffer> *local = new std::vector<RtMidiBuffer>(*midiLocalBuffers);
        std::vector<RtMidiQueue> *inMap = new std::vector<RtMidiQueue>(*midiInMap);
        std::vector<RtMidiOutput> *outMap = new std::vector<RtMidiOutput>(*midiOutMap); 

        RtMidiQueue inq;
        inq.id = port->id();
        inq.queue = port->inQueue();
        inMap->push_back(inq);

        RtMidiOutput outq(port, port->id());
        outMap->push_back(outq);

        RtMidiBuffer locq;
        locq.id = port->id();
        locq.buffer = port->rtLocalBuffer();
        local->push_back(locq);

        port->portsAddedToRt();

        FlatEvents::FlatControl ctrl;
        ctrl.type = FlatEvents::FlatControl::Type::UpdateMidiMaps;
        ctrl.commandId = ControlEngine::generateCommandId();
        ctrl.updateMidiMaps.engine = engine;
        ctrl.updateMidiMaps.inputMap = inMap;
        ctrl.updateMidiMaps.outputMap = outMap;
        ctrl.updateMidiMaps.localBuffers = local;
        ControlEngine::emitRtControl(ctrl);
    }

#if (USE_PUSH == 1 && USE_FAKE_PUSH == 1)
    LOG_WARN("FAKING PUSH, DISABLE ON RASPBERRY");
    PushThread::start(nullptr);
#endif            
}

MidiController::~MidiController() {
#if (USE_PUSH == 1)
    PushThread::shutdown();
#endif

    for(std::unique_ptr<MidiPort> &port : _activePorts) {
        if(port->id() != 0) 
            closeDevice(port.get());
        // delete port;
    }

    //close virtual midi

}

void MidiController::checkDevices() {
    //this is separate, independent thread
    std::vector<MidiDevice> list = discoverMidiDevices();

    std::lock_guard<std::mutex> l(_mutex);

    for(std::unique_ptr<MidiDevice> &exist : _deviceList) {
        exist->_check = false;
    }

    for(MidiDevice &devFound : list) {
        std::vector<std::unique_ptr<MidiDevice>>::iterator it;
        it = std::find_if(
            _deviceList.begin(), 
            _deviceList.end(),
            [&](const std::unique_ptr<MidiDevice> &exist) {
                return (exist->_card == devFound._card && 
                        exist->_name == devFound._name);
            }
        );

        if(it != _deviceList.end()) {
            if(!it->get()->_online) {
                //device got back
                it->get()->_online = true;
                LOG_INFO("Device connected back %d", it->get()->_card);
            }
            it->get()->_check = true;
        } else {
            //new device
            devFound._check = true;
            devFound._online = true;
            std::unique_ptr<MidiDevice> d = std::make_unique<MidiDevice>(devFound);
            MidiDevice * dev = d.get();

            _deviceList.push_back(std::move(d));
            LOG_INFO("New device %d", devFound._card);
#if (USE_PUSH == 1)
            if(dev->_name.compare("Ableton Push 2") == 0) {
                //...
                //need to get everything from here to similar an for GUI - pushThread.cpp with only start() func?
                LOG_INFO("Ableton device found!"); 

                MidiSubdevice * subdevptr = &dev->_ports.at(0); //get the Live port not User

                std::unique_ptr<MidiPort> pushPort = std::make_unique<MidiPort>();
                MidiPort *port = pushPort.get();
                port->_controller = this;
                port->_ownerDev = dev;
                port->_ownerSubdev = subdevptr;
                port->_path = subdevptr->_path;

                _activePorts.push_back(std::move(pushPort));
                PushThread::start(port);
            }
#endif
            
        }
    }

    for(std::unique_ptr<MidiDevice> &dev : _deviceList) {
        if(!dev->_check) {
            if(dev->_online) { //already there
                dev->_online = false;
                LOG_INFO("Device disconnected %i", dev->_card);
            }       
        }
    }
}

std::vector<MidiDevice> MidiController::devList() {
    std::vector<MidiDevice> ret;
    
    {
        std::lock_guard<std::mutex> lock(_mutex);
        for(auto &dev : _deviceList) {
            MidiDevice d(*dev.get());
            ret.push_back(d);
        }
        // ret = _deviceList;
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
    // port->_anchorTimepoint = _midiAnchorTimepoint;

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

void MidiController::addVirtualKbdEvent(const MidiEvent ev) {
    MidiPort * port = _activePorts.at(0).get();
    port->pushEvent(ev.type, ev.channel, ev.note, ev.velocity);
}

}