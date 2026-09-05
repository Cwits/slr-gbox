// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "push/core/PushCore.h"

#include "core/primitives/MidiEvent.h"
#include "core/primitives/MidiPort.h"
#include "core/MidiController.h"

#include "push/core/Widget.h"
#include "push/core/BoundingBox.h"
#include "push/core/PushMidi.h"

#include "common/logger.h"
#include "common/core_config.h"

#include <thread>
#include <atomic>
#include <future>
#include <chrono>
#include <cmath>
#include <vector>

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
    _rootWidget(nullptr),
    _manualRedraw(false)
{
    _context._core = this;
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
    if(_connected)
        _display.disconnect();
    #endif
}

void PushCore::tick(int dt) {
    if(!_rootWidget) {
        LOG_ERROR("Root widget is not set");
        return;
    }

    /*bool needRedraw = */_midi.dispatchEvents();

    //animations tick?
    if(_pads.needUpdate()) {
        _pads.update();
    }
    
    bool needRedraw = _rootWidget->hasAnythingDirty(); 

    if(needRedraw || _manualRedraw) {
        if(!_rootWidget) { 
            LOG_ERROR("Root Widget is not set");
            return;
        }

        _rootWidget->paint(_painter);
        
        BoundingBox dirtyRegion = BoundingBox(0, 0, 0, 0);//_rootWidget->bounds();
        _display.updateFrame(dirtyRegion);
        _manualRedraw = false;
    }

    if(!_display.sendFrame()) 
        _connected = false;
}

}