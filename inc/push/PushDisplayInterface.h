// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/PushLib.h"
// #include "libusb.h"
#include <libusb-1.0/libusb.h>

#include <functional>
#include <memory>

namespace PushLib {

class Widget;
class Painter;

struct DisplayInterface {
    enum class DisplaySysex : char {
        SET_BRIGHTNESS = 0x08,
        GET_BRIGHTNESS = 0x09
    };

    DisplayInterface(/*SysexInterface &sysex*/Painter & painter);
    ~DisplayInterface();

    bool connect();
    bool reconnect();
    bool disconnect();

    void updateFrame(/* Pixel buffer */);
    bool sendFrame();

    void setBrignthess(char brightness);
    char getBrightness();

    private:
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