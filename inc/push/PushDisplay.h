// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/PushLib.h"

#include <libusb-1.0/libusb.h>

#include <functional>
#include <memory>

namespace PushLib {

class Widget;
class Painter;
class PushSysex;
class BoundingBox;

struct PushDisplay {
    PushDisplay(PushSysex &sysex, Painter & painter);
    ~PushDisplay();

    bool connect();
    bool reconnect();
    bool disconnect();

    void updateFrame(BoundingBox &boxToUpdate);
    bool sendFrame();

    void setBrignthess(char brightness);
    char getBrightness();

    private:
    PushSysex &_sysex;
    Painter & _painter;

    using devHandlePtr = std::unique_ptr<libusb_device_handle,
                            std::function<void(libusb_device_handle *)>>;

    /* SysexInterface & sysex; */
    devHandlePtr _displayHandle;

    std::unique_ptr<unsigned char> _usbFrame;
    // std::unique_ptr<unsigned char> _frameBuffer2;

    Widget * _rootWidget;
};

}