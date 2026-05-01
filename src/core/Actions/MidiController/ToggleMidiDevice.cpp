// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Actions/MidiController/ToggleMidiDevice.h"

#include "core/primitives/ActionBase.h"
#include "core/primitives/ControlContext.h"

#include "core/Project.h"
#include "core/RtEngine.h"

#include "snapshots/ProjectView.h"

#include "ui/uiControls.h"

#include "logger.h"

#include <cassert>

namespace slr {

ToggleMidiDeviceAction::ToggleMidiDeviceAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::ToggleMidiDevice*>(base)) )
{

}

ToggleMidiDeviceAction::~ToggleMidiDeviceAction() {

}

void ToggleMidiDeviceAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    switch(_step) {
    	case(1): {
            //check that device exists
            // std::vector<MidiDevice> list = ctx.midiController->devList();
            // MidiDevice * device = nullptr;

            // for(MidiDevice &dev : list) {
            //     int pathcard = e.path[3] - 48;
            //     if(dev._card != pathcard) continue;

            //     device = &dev;
            //     break;
            // }

            if(!_action.device) {
                //no such card
                LOG_ERROR("No such midi device %s to enable", _action.device->_name.c_str());
                
                if(_action.completed)
                    _action.completed(1);
                
                abortAction();
                return;
            }

            if(!_action.device->_online) {
                LOG_ERROR("Midi device %s is offline", _action.device->_name.c_str());
                
                if(_action.completed)
                    _action.completed(2);

                abortAction();
                return;
            }

            // MidiSubdevice *sub = nullptr;
            // for(MidiSubdevice &dev : device->_ports) {
            //     if(e.path != dev._path) continue;
                
            //     sub = &dev;
            //     break;
            // }

            if(!_action.subdev) {
                LOG_ERROR("Card %s doesn't have specified subdevice", _action.subdev->_path.c_str());
                
                if(_action.completed)
                    _action.completed(3);

                abortAction();
                return;
            }

            if(_action.port == DevicePort::INPUT && !_action.subdev->_hasInput) {
                LOG_ERROR("Card %s input can't be enabled because it doesn't have one", _action.subdev->_path.c_str());
                
                if(_action.completed)
                    _action.completed(4);

                abortAction();
                return;
            }

            if(_action.port == DevicePort::OUTPUT && !_action.subdev->_hasOutput) {
                LOG_ERROR("Card %s output can't be enabled because it doesn't have one", _action.subdev->_path.c_str());

                if(_action.completed)
                    _action.completed(5);

                abortAction();
                return;
            }

            //check that not already enabled in desired combination
            std::vector<std::unique_ptr<MidiPort>> &activePorts = ctx.midiController->activePorts();
            MidiPort * port = nullptr;
            for(std::unique_ptr<MidiPort> &active : activePorts) {
                if(active->_ownerDev->_name.compare(_action.device->_name) == 0 &&
                    (active->_ownerSubdev->_inputName.compare(_action.subdev->_inputName) == 0 ||
                    active->_ownerSubdev->_outputName.compare(_action.subdev->_outputName))
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

                port->_path = _action.subdev->_path;
                port->_ownerDev = _action.device;
                port->_ownerSubdev = _action.subdev;
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
            if(_action.port == DevicePort::INPUT) {
                ctx.midiController->openDevice(port, _action.newState, port->outputOpened());
            } else if(_action.port == DevicePort::OUTPUT) {
                ctx.midiController->openDevice(port, port->inputOpened(), _action.newState);
            }

            if(_action.newState && !port->isOpened()) {
                LOG_ERROR("Failed to open %s port with %s %s",
                    _action.subdev->_path.c_str(),
                    (_action.port == DevicePort::INPUT ? "input" : "output"),
                    (_action.newState ? "open" : "close")
                );

                if(_action.completed)
                    _action.completed(6);

                abortAction();
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

                _flat.engine = ctx.engine;
                _flat.inputMap = inMap;
                _flat.outputMap = outMap;
                _flat.localBuffers = local;
                _flat.oldLocal = nullptr;
                _flat.oldInput = nullptr;
                _flat.oldOutput = nullptr;

                _flat.completed.store(false);
                _task = makeRtTask(&_flat);

                setState(ActionState::Waiting);
                ctx.EmitRtTask(&_task);
            } else {
                markDelete();
                setState(ActionState::Finished);
            }

            if(_action.completed)
                _action.completed(0);
        } break;
    	case(2): {
            if(!_flat.oldLocal || !_flat.oldInput || !_flat.oldOutput) {
                LOG_ERROR("Smth went wrong");
                abortAction();
                return;
            }

            LOG_INFO("Midi Maps swapped successfully");
            delete _flat.oldLocal;
            delete _flat.oldInput;
            delete _flat.oldOutput;

            markDelete();
            setState(ActionState::Finished);
    	} break;
        default: assert(false && "Unreachable"); break;
    }
}

void ToggleMidiDeviceAction::checkWaitingCondition() {
    assert(getState() == ActionState::Waiting);

    switch(_step) {
        case(1): {
            bool res = _flat.completed.load(std::memory_order_acquire);
            if(res) {
                _step = 2;
                setState(ActionState::Executing);
            }
        } break;
        default: assert(false && "Unreachable"); break;
    }
}


void ToggleMidiDeviceAction::UpdateMidiMaps::execRT() {
    oldLocal = engine->midiLocalBuffers();
    oldInput = engine->midiInMap();
    oldOutput = engine->midiOutMap();

    engine->setMidiLocal(localBuffers);
    engine->setMidiIn(inputMap);
    engine->setMidiOut(outputMap);

    completed.store(true, std::memory_order_release);
}

std::unique_ptr<ActionExecutable> createToggleMidiDeviceAction(const ActionBase *base) {
    return std::make_unique<ToggleMidiDeviceAction>(base);
}

}