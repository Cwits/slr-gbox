// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "push/PushDisplay.h"
// #include "push/widgets/Widget.h"
#include "push/PushPainter.h"
#include "push/PushSysex.h"

#include "logger.h"
#include "slr_config.h"

#include <cstring>

#if (USE_FAKE_PUSH == 1)
#include "push/PushFaker.h"
extern PushLib::PushFaker _faker;
#endif

namespace PushLib {
/* The Push 2 Display shows 160 lines of 960 pixels of 16 bit each. 
    The pixel data is sent line by line, starting with the topmost. 
    For each line, 2kbytes are sent. 
    They consist of 1920 bytes of pixel data and 128 filler bytes, 
    which avoid line borders occuring in the middle of the 512 byte USB buffers. 
    The pixels for each line are sent with leftmost pixel first.     
*/

//2 bytes per pixel
constexpr int DISPLAY_WIDTH_BYTES = DISPLAY_WIDTH * 2;
constexpr int DISPLAY_PADDING_BYTES = 128;

constexpr int ROW_LENGTH = DISPLAY_WIDTH_BYTES + DISPLAY_PADDING_BYTES;
//why *2??
constexpr int FRAME_BUFFER_LENGTH = (ROW_LENGTH * (DISPLAY_HEIGHT/**2*/));

using devListPtr = std::unique_ptr<libusb_device *, std::function<void(libusb_device **)>>;

libusb_device_handle * findDevice(unsigned int PRODUCT_ID, 
                                  unsigned int VENDOR_ID);
libusb_device ** getDeviceList();

constexpr unsigned int ALBETON_VENDOR_ID = 0x2982;
constexpr unsigned int ABLETON_PRODUCT_ID = 0x1967;

constexpr unsigned char frame_header[16] = { 0xFF, 0xCC, 0xAA, 0x88, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00 };

constexpr unsigned char signal_shaping_pattern_len = 4;
constexpr unsigned char signal_shaping_pattern[signal_shaping_pattern_len] = {
    0xE7, 0xF3, 0xE7, 0xFF};

constexpr unsigned char PUSH2_BULK_EP_OUT = 0x01;
constexpr unsigned int TRANSFER_TIMEOUT = 500;

enum DisplaySysex : unsigned char {
    SET_BRIGHTNESS = 0x08,
    GET_BRIGHTNESS = 0x09
};

PushDisplay::PushDisplay(PushSysex &sysex, Painter & painter) : _sysex(sysex), _painter(painter)
{
    _sysex.registerCmdWithReply(SET_BRIGHTNESS);
    _sysex.registerCmdWithReply(GET_BRIGHTNESS);
}

PushDisplay::~PushDisplay() {

}

bool PushDisplay::connect() {
    
#if (USE_FAKE_PUSH == 0)
    if(_displayHandle) return false;

    int res = 0;
    if((res = libusb_init(NULL)) < 0) {
        LOG_ERROR("Failed to init libusb for Push");
        return false;
    }

    _displayHandle = std::unique_ptr<libusb_device_handle,
                                    std::function<void(libusb_device_handle*)>>
                        (
                        findDevice(ABLETON_PRODUCT_ID, ALBETON_VENDOR_ID),
                        [](libusb_device_handle *handle) {
                            libusb_release_interface(handle, 0);
                            libusb_close(handle);
                        });

    if(!_displayHandle) {
        LOG_ERROR("Failed to find or init Push Device");
        return false;
    } else {
#endif
        _usbFrame = std::unique_ptr<unsigned char>(new unsigned char[FRAME_BUFFER_LENGTH]);

        if(!_usbFrame /*|| !_frameBuffer2*/) {
            LOG_ERROR("Failed to allocate memory for frame buffers");
            return false;//?? TODO: need proper clean up
        }

        std::memset(_usbFrame.get(), 0, FRAME_BUFFER_LENGTH*sizeof(unsigned char));
        BoundingBox b;
        b.x = 0;
        b.y = 0;
        b.w = DISPLAY_WIDTH;
        b.h = DISPLAY_HEIGHT;
        updateFrame(b);
#if (USE_FAKE_PUSH == 0)
    }
#endif

    return true;
}

bool PushDisplay::reconnect() {
    return true;
}

bool PushDisplay::disconnect() {
    if(_displayHandle) {
        _displayHandle.reset(nullptr);
    }
    return true;
}

void PushDisplay::updateFrame(BoundingBox &boxToUpdate) {
   
    //do some time calculations...??
    //...

    //if have nothing to redraw - skip
    // if(!_rootWidget->isDirty()) return;

    //draw pieces content
    //...

    
    //fill tmp
    const Pixel * canvas = _painter.canvas();
    unsigned char * fbPtr = _usbFrame.get();
    for(int x=0; x<DISPLAY_WIDTH; ++x) {

        for(int y=0; y<DISPLAY_HEIGHT; ++y) {
            int pixIdx = y * DISPLAY_WIDTH + x;
            Pixel px = canvas[pixIdx];

            unsigned char MSB = px >> 8;
            unsigned char LSB = px & 0x00FF;

            int pattr_index = x * 2;

            LSB ^= signal_shaping_pattern[(pattr_index) % signal_shaping_pattern_len];
            MSB ^= signal_shaping_pattern[(pattr_index + 1) % signal_shaping_pattern_len];

            int idx = y * ROW_LENGTH + x * 2;
            fbPtr[idx]     = LSB;
            fbPtr[idx + 1] = MSB;
        }
    }

    // for(int row = 0; row < DISPLAY_HEIGHT; ++row) {
    //     for(int col = DISPLAY_WIDTH_BYTES; col < ROW_LENGTH; ++col) {
    //         fbPtr[row * ROW_LENGTH + col] = 0x00;
    //     }
    // }
}

bool PushDisplay::sendFrame() {
#if (USE_FAKE_PUSH == 0)
    int result = libusb_bulk_transfer(_displayHandle.get(), PUSH2_BULK_EP_OUT,
                                        frame_header, sizeof(frame_header), NULL,
                                        TRANSFER_TIMEOUT);


    if(result != 0) return false;

    unsigned char * fbPtr = _usbFrame.get();
    result = libusb_bulk_transfer(_displayHandle.get(), PUSH2_BULK_EP_OUT, fbPtr,
                                    FRAME_BUFFER_LENGTH, NULL, TRANSFER_TIMEOUT);

    if(result != 0) return false;
#else
    unsigned char * fbPtr = _usbFrame.get();
    _faker.sendCmd(PushFaker::FakerCmd::DisplayFrame, fbPtr, FRAME_BUFFER_LENGTH);
#endif
    
    return true;
}

void PushDisplay::setBrignthess(char brightness) {

}

char PushDisplay::getBrightness() {
    return 0;
}

libusb_device_handle * findDevice(unsigned int PRODUCT_ID,
                                                    unsigned int VENDOR_ID) {
    devListPtr devices(getDeviceList(), [](libusb_device **deviceList) {
        libusb_free_device_list(deviceList, 1);
    });

    libusb_device_handle *devHandle;
    libusb_device *dev;
    int result;

    for(int i=0; (dev = devices.get()[i]) != NULL; ++i) {
        struct libusb_device_descriptor descriptor;
        
        if( (result = libusb_get_device_descriptor(dev, &descriptor)) < 0 ) {
            continue;
        }
        if(descriptor.bDeviceClass == LIBUSB_CLASS_PER_INTERFACE &&
            descriptor.idVendor == VENDOR_ID && 
            descriptor.idProduct == PRODUCT_ID) {
                if( (result = libusb_open(dev, &devHandle)) < 0) {
                    return nullptr;
                } else if( (result = libusb_claim_interface(devHandle, 0)) < 0) {
                    libusb_close(devHandle);
                    devHandle = NULL;
                    return nullptr;
                } else {
                    break; //successfully opened
                }
            }
    }

    return devHandle;
}

libusb_device ** getDeviceList() {
    libusb_device ** devices;
    ssize_t count = libusb_get_device_list(NULL, &devices);

    if(count < 0) {
        return nullptr;
    }

    return devices;
}

}