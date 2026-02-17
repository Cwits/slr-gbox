// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

START_BLOCK

EV
INCLUDE <string>
INCLUDE <functional>
INCLUDE "core/MidiController.h"
struct ToggleMidiDevice {
    MidiDevice * device;
    MidiSubdevice * subdev;
    DevicePort port;
    bool newState;
    std::function<void(int)> completed;
};

FLAT_REQ
INCLUDE <unordered_map>
INCLUDE "defines.h"
struct UpdateMidiMaps {
    RtEngine * engine; //-> class RtEngine;
    std::vector<RtMidiBuffer> *localBuffers;
    std::vector<RtMidiQueue> *inputMap;
    std::vector<RtMidiOutput> *outputMap;
};

FLAT_RESP
INCLUDE <unordered_map>
INCLUDE "Status.h"
INCLUDE "defines.h"
struct UpdateMidiMaps {
    std::vector<RtMidiBuffer> *oldLocal;
    std::vector<RtMidiQueue> *oldInput;
    std::vector<RtMidiOutput> *oldOutput;
    Status status;
};

EV_HANDLE
INCLUDE "core/MidiController.h"
INCLUDE "core/ControlEngine.h"
INCLUDE "core/RtEngine.h"
INCLUDE "core/primitives/MidiEvent.h" //for RtMidiPort
INCLUDE "logger.h"
INCLUDE <vector>
INCLUDE <unordered_map>
INCLUDE <array>
void handleEvent(const ControlContext &ctx, const Events::ToggleMidiDevice &e) {
    //check that device exists
    // std::vector<MidiDevice> list = ctx.midiController->devList();
    // MidiDevice * device = nullptr;

    // for(MidiDevice &dev : list) {
    //     int pathcard = e.path[3] - 48;
    //     if(dev._card != pathcard) continue;

    //     device = &dev;
    //     break;
    // }

    if(!e.device) {
        //no such card
        LOG_ERROR("No such midi device %s to enable", e.device->_name.c_str());
        
        if(e.completed)
            e.completed(1);
        return;
    }

    if(!e.device->_online) {
        LOG_ERROR("Midi device %s is offline", e.device->_name.c_str());
        
        if(e.completed)
            e.completed(2);
        return;
    }

    // MidiSubdevice *sub = nullptr;
    // for(MidiSubdevice &dev : device->_ports) {
    //     if(e.path != dev._path) continue;
        
    //     sub = &dev;
    //     break;
    // }

    if(!e.subdev) {
        LOG_ERROR("Card %s doesn't have specified subdevice", e.subdev->_path.c_str());
        
        if(e.completed)
            e.completed(3);
        return;
    }

    if(e.port == DevicePort::INPUT && !e.subdev->_hasInput) {
        LOG_ERROR("Card %s input can't be enabled because it doesn't have one", e.subdev->_path.c_str());
        
        if(e.completed)
            e.completed(4);
        return;
    }

    if(e.port == DevicePort::OUTPUT && !e.subdev->_hasOutput) {
        LOG_ERROR("Card %s output can't be enabled because it doesn't have one", e.subdev->_path.c_str());

        if(e.completed)
            e.completed(5);
        return;
    }

    //check that not already enabled in desired combination
    std::vector<std::unique_ptr<MidiPort>> &activePorts = ctx.midiController->activePorts();
    MidiPort * port = nullptr;
    for(std::unique_ptr<MidiPort> &active : activePorts) {
        if(active->_ownerDev->_name.compare(e.device->_name) == 0 &&
            (active->_ownerSubdev->_inputName.compare(e.subdev->_inputName) == 0 ||
            active->_ownerSubdev->_outputName.compare(e.subdev->_outputName))
            ) {
                port = active.get();
                break;
            }
    }

    if(!port) {
        //port doesn't exist -> create one
        std::unique_ptr<MidiPort> tmp = std::make_unique<MidiPort>();
        port = tmp.get();
        ctx.midiController->addNewPort(std::move(tmp));
        // TODO: ctx.midiController->addNewPort(e.device, e.subdev);

        port->_path = e.subdev->_path;
        port->_ownerDev = e.device;
        port->_ownerSubdev = e.subdev;
    }

    // if(e.port == DevicePort::INPUT) {
    //     if(e.newState && port->inputOpened()) {
    //         LOG_WARN("Input for device %s already opened", e.device->_name);
    //         return;
    //     }
    // }
    // if(e.port == DevicePort::OUTPUT) {
    //     if(e.newState && port->outputOpened()) {
    //         LOG_WARN("Output for device %s already opened", e.device->_name);
    //         return;
    //     }
    // }

    //enable
    if(e.port == DevicePort::INPUT) {
        ctx.midiController->openDevice(port, e.newState, port->outputOpened());
    } else if(e.port == DevicePort::OUTPUT) {
        ctx.midiController->openDevice(port, port->inputOpened(), e.newState);
    }

    if(e.newState && !port->isOpened()) {
        LOG_ERROR("Failed to open %s port with %s %s",
            e.subdev->_path.c_str(),
            (e.port == DevicePort::INPUT ? "input" : "output"),
            (e.newState ? "open" : "close")
        );

        if(e.completed)
            e.completed(6);
        return;
    }



    //pass queues to RT
    //check that RT Engine doesn't have queues for this specific port

    const ID portId = port->id();
    if(port->justCreated()) {
        const std::vector<RtMidiBuffer> *midiLocalBuffers = ctx.engine->midiLocalBuffers();
        const std::vector<RtMidiQueue> *midiInMap = ctx.engine->midiInMap();
        const std::vector<RtMidiOutput> *midiOutMap = ctx.engine->midiOutMap();

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
        ctrl.updateMidiMaps.engine = ctx.engine;
        ctrl.updateMidiMaps.inputMap = inMap;
        ctrl.updateMidiMaps.outputMap = outMap;
        ctrl.updateMidiMaps.localBuffers = local;
        ControlEngine::emitRtControl(ctrl);
    }

    if(e.completed)
        e.completed(0);
}
END_HANDLE

RT_HANDLE
INCLUDE "core/RtEngine.h"
&RtEngine::updateMidiMaps
END_HANDLE

RESP_HANDLE
INCLUDE "Status.h"
INCLUDE "logger.h"
void handleUpdateMidiMapsResponse(const ControlContext &ctx, const FlatEvents::FlatResponse &resp) {
    if(resp.status == Status::Ok) {
        LOG_INFO("Midi Maps swapped successfully");
        delete resp.updateMidiMaps.oldInput;
        delete resp.updateMidiMaps.oldOutput;
        delete resp.updateMidiMaps.oldLocal;
    }
}
END_HANDLE

END_BLOCK
