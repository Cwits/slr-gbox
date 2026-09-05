// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <functional>
#include <thread>
#include <atomic>

namespace PushLib {

struct __attribute__((packed)) command {
    unsigned char cmd;
    uint32_t dataSize;
    uint8_t reserved[3];
};

struct PushFaker {
    enum class FakerCmd {
        ERR = 0,
        DisplayFrame = 1,
        Button = 2,
        Encoder = 3,
        Touchstrip = 4,
        Pedal = 5,
        Pad = 6,
        System = 7
    };

    ~PushFaker();

    void init();
    void deinit();

    void sendCmd(const FakerCmd cmd, const unsigned char * data, uint32_t dataSize);
    
    std::function<void(const unsigned char)> realTimeHandle;
    std::function<void(const unsigned char *, const int &)> sysexHandle;
    std::function<void(const unsigned char, const int, const unsigned char *)> eventHandle;

    private:
    int _sock;
    sockaddr_in _addr;
    bool _connected;
    
    void sendData(const unsigned char * data, int size);

    std::thread _rxThread;
    std::atomic<bool> _rxRunning;

    static void rxLoop(PushFaker *faker);
    void onPacket(command &cmd, const std::vector<unsigned char>& payload);

};

    
}