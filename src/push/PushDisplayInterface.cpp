// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "push/PushDisplayInterface.h"

#include "logger.h"

#include <cstring>

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

// using Pixel = DisplayInterface::Pixel;
// using devHandlePtr = DisplayInterface::devHandlePtr;
using devListPtr = std::unique_ptr<libusb_device *, std::function<void(libusb_device **)>>;

libusb_device_handle * findDevice(unsigned int PRODUCT_ID, 
                                  unsigned int VENDOR_ID);
libusb_device ** getDeviceList();

constexpr unsigned int ALBETON_VENDOR_ID = 0x2982;
constexpr unsigned int ABLETON_PRODUCT_ID = 0x1967;

unsigned char frame_header[16] = { 0xFF, 0xCC, 0xAA, 0x88, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00 };

constexpr unsigned char signal_shaping_pattern_len = 4;
constexpr unsigned char signal_shaping_pattern[signal_shaping_pattern_len] = {
    0xE7, 0xF3, 0xE7, 0xFF};

constexpr unsigned char PUSH2_BULK_EP_OUT = 0x01;
constexpr unsigned int TRANSFER_TIMEOUT = 500;

uint16_t fb[DISPLAY_HEIGHT][DISPLAY_WIDTH];

DisplayInterface::DisplayInterface(/*SysexInterface &sysex*/) {

}

DisplayInterface::~DisplayInterface() {

}

bool DisplayInterface::connect() {
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
        _frameBuffer1 = std::unique_ptr<unsigned char>(new unsigned char[FRAME_BUFFER_LENGTH]);

        if(!_frameBuffer1 /*|| !_frameBuffer2*/) {
            LOG_ERROR("Failed to allocate memory for frame buffers");
            return false;//?? TODO: need proper clean up
        }

        std::memset(_frameBuffer1.get(), 0, FRAME_BUFFER_LENGTH*sizeof(unsigned char));
    }

    return true;
}

bool DisplayInterface::reconnect() {
    return true;
}

bool DisplayInterface::disconnect() {
    if(_displayHandle) {
        _displayHandle.reset(nullptr);
    }
    return true;
}

void DisplayInterface::updateFrame(/* Pixel buffer */) {

}

bool DisplayInterface::sendFrame() {
    int result = libusb_bulk_transfer(_displayHandle.get(), PUSH2_BULK_EP_OUT,
                                        frame_header, sizeof(frame_header), NULL,
                                        TRANSFER_TIMEOUT);
    
    if(result != 0) return false;
    
    static int coloridx = 0;
    
    uint16_t color = 0;
    {
        int r, g, b;
        if(coloridx == 0) {
            r = 255; g = 0; b = 0;
            coloridx++;
        } else if(coloridx == 1) {
            r = 0; g = 255; b = 0;
            coloridx++;
        } else if(coloridx == 2) {
            r = 0; g = 0; b = 255;
            coloridx = 0;
        }
        uint16_t tmpr, tmpg, tmpb;
        tmpr = tmpg = tmpb = 0;

        if(r>0) tmpr = 0 + (float)((float)(0x001F-0)/(float)(255-0)) * (r - 0);
        if(g>0) tmpg = 0x001F + (float)((float)(0x07E0 - 0x001F)/(float)(255-0)) * (g - 0);
        if(b>0) tmpb = 0x07E0 + (float)((float)(0xF800 - 0x07E0)/(float)(255-0)) * (b - 0);

        color = tmpr | tmpg | tmpb;
    }

    for(int x=0; x<DISPLAY_WIDTH; ++x) {
        for(int y=0; y<DISPLAY_HEIGHT; ++y) {
            fb[y][x] = color;
        }
    }

    //fill tmp
    unsigned char * fbPtr = _frameBuffer1.get();
    for(int i=0; i<DISPLAY_HEIGHT; ++i) {
        int pattr_index = 0;

        for(int y=0; y<DISPLAY_WIDTH; ++y) {
            unsigned char MSB = fb[i][y] >> 8;
            unsigned char LSB = fb[i][y] & 0x00FF;

            LSB ^= signal_shaping_pattern[pattr_index++ % signal_shaping_pattern_len];
            MSB ^= signal_shaping_pattern[pattr_index++ % signal_shaping_pattern_len];

            int idx = (i * ROW_LENGTH) + y * 2;
            fbPtr[idx] = LSB;
            fbPtr[idx + 1] = MSB;
        }

        for(int col=DISPLAY_WIDTH_BYTES; col<ROW_LENGTH; ++col) {
            fbPtr[i * ROW_LENGTH + col] = 0x00;
        }
    }


    result = libusb_bulk_transfer(_displayHandle.get(), PUSH2_BULK_EP_OUT, fbPtr,
                                    FRAME_BUFFER_LENGTH, NULL, TRANSFER_TIMEOUT);

    if(result != 0) return false;
    return true;
}

void DisplayInterface::setBrignthess(char brightness) {

}

char DisplayInterface::getBrightness() {
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