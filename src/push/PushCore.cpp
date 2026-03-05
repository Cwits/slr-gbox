// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "push/PushCore.h"

#include "core/primitives/MidiPort.h"
#include "core/MidiController.h"
#include "core/Events.h"

#include "push/Widget.h"

#include "logger.h"
#include "slr_config.h"

#include <thread>
#include <atomic>
#include <future>
#include <chrono>
#include <cmath>

#if (USE_FAKE_PUSH == 1)
#include "push/PushFaker.h"
PushLib::PushFaker _faker;
#endif


namespace PushLib {

PushCore::PushCore() :
    _midi(*this, _sysex),
    _sysex(_midi),
    _display(_sysex, _painter),
    _leds(_midi, _sysex),
    _pads(_midi, _sysex, _leds),
    _mainWidget(nullptr),
    _manualRedraw(false)
{
    _context._leds = &_leds;
    _context._pads = &_pads;
}

PushCore::~PushCore() {

}

bool PushCore::connect(slr::MidiPort * port) {
    _connected = false;
    if(!_midi.connect(port)) {
        return false;
    }
    if(!_display.connect()) {
        return false;
    }
    
    _connected = true;
    return _connected;
}

void PushCore::reconnect() {
    
}

void PushCore::disconnect() {
    #if (USE_FAKE_PUSH == 0)
    _display.disconnect();
    #endif
}

void PushCore::tick(int dt) {
    bool needRedraw = _midi.dispatchEvents();

    //animations tick?
    if(_pads.needUpdate()) {
        _pads.update();
    }

    //do i need to update buttons colors corresponding to current view here? or 
    //view can do that by himself?
    
    if(needRedraw || _manualRedraw) {
        if(!_mainWidget) { 
            LOG_ERROR("Main Widget is not set");
            return;
        }

        BoundingBox box = _mainWidget->invalidate();
        _mainWidget->paint(_painter);
        _display.updateFrame(box);
        _manualRedraw = false;
    }

    if(!_display.sendFrame()) 
        _connected = false;
}

}