// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "push/PushFaker.h"

#include "logger.h"

#include <iostream>
#include <vector>
#include <cstring>
#include <thread>
#include <chrono>
#include <errno.h>


namespace PushLib {

static const int   TARGET_PORT = 9000;

bool recv_exact(int sock, void* buf, int size)
{
    char* p = (char*)buf;
    int got = 0;

    while(got < size)
    {
        int n = recv(sock, p + got, size - got, 0);
        if(n <= 0) return false;
        got += n;
    }
    return true;
}

PushFaker::~PushFaker() {
    deinit();
    if(_sock) close(_sock);
}

void PushFaker::init() {
    _sock = socket(AF_INET, SOCK_STREAM, 0);
    
    _addr = sockaddr_in{};
    _addr.sin_family = AF_INET;
    _addr.sin_port   = htons(TARGET_PORT);
    inet_pton(AF_INET,"127.0.0.1",&_addr.sin_addr);

    if(connect(_sock,(sockaddr*)&_addr,sizeof(_addr))<0){
        std::cout<<"connect failed\n";
        _connected = false;
        return;
    }

    _connected = true;

    _rxThread = std::thread(&PushFaker::rxLoop, this);
}

void PushFaker::deinit() {
    _rxRunning = false;
    ::shutdown(_sock, SHUT_RDWR);
    if(_rxThread.joinable())
        _rxThread.join();
}

void PushFaker::sendCmd(const FakerCmd cmd, const unsigned char * data, uint32_t dataSize) {
    command c;
    c.cmd = static_cast<unsigned char>(cmd);
    c.dataSize = dataSize;
    c.reserved[0] = 0xAD;
    c.reserved[1] = 0xBE;
    c.reserved[2] = 0xCF;

    sendData((unsigned char *)&c, sizeof(command));
    sendData(data, dataSize);
    unsigned char end[3];
    end[0] = 0xAD;
    end[1] = 0xBE;
    end[2] = 0xCF;
    sendData((unsigned char *)&end, 3);
}

void PushFaker::sendData(const unsigned char * data, int size) {
    if(!_sock || !_connected) {
        LOG_ERROR("Sock is not inited");   
        return;
    }

    int err = send(_sock, data, size, 0);
    if(err < 0) {
        LOG_INFO("%d ret val: %d", _sock, err);
        std::cout << "sendto failed: " << strerror(errno) << "\n";
    }
}

void PushFaker::rxLoop(PushFaker *faker) {
    faker->_rxRunning = true;

    while(faker->_rxRunning) {
        command cmd;
        if(!recv_exact(faker->_sock, &cmd, sizeof(cmd))) {
            LOG_WARN("Failed to rcv data");
            faker->_rxRunning = false;
            continue;
        }


        std::vector<unsigned char> payload(cmd.dataSize);

        if(!recv_exact(faker->_sock, payload.data(), cmd.dataSize)) {
            LOG_WARN("Failed to rcv data");
            faker->_rxRunning = false;
            continue;
        }

        faker->onPacket(cmd, payload);
    }
}

void PushFaker::onPacket(command &cmd, const std::vector<unsigned char>& payload) {
    if(cmd.cmd == static_cast<unsigned char>(PushFaker::FakerCmd::Button)) {
        // if(payload[1] == 0) {
        //     LOG_INFO("Button %d released", (int)payload[0]);
        //     // std::cout << "Button " << (int)payload[0] << " released";
        // } else {
        //     LOG_INFO("Button %d pressed", (int)payload[0]);
        //     // std::cout << "Button " << (int)payload[0] << " pressed";
        // }
        unsigned char data[2];
        data[0] = payload[0]; //button number
        data[1] = payload[1] ? 1 : 0; //state
        eventHandle(0xB0, 0, &data[0]);
    } else if(cmd.cmd == static_cast<unsigned char>(PushFaker::FakerCmd::Pad)) {
        // if(payload[1] == 0) {
        //     LOG_INFO("Pad %d released", (int)payload[0]);
        //     // std::cout << "Pad " << (int)payload[0] << " released";mak
        // } else {
        //     LOG_INFO("Pad %d pressed", (int)payload[0]);
        //     // std::cout << "Pad " << (int)payload[0] << " pressed";
        // }
        unsigned char data[2];
        data[0] = payload[0]; //pad number
        data[1] = 120; //velocity todo: generate random value
        eventHandle(payload[1] ? 0x90 : 0x80, 0, &data[0]);
    } else if(cmd.cmd == static_cast<unsigned char>(PushFaker::FakerCmd::Encoder)) {
        //some encoder data
        
        // payload[0] = id;    //enc id
        // payload[1] = delta; //value
        // payload[2] = state; //touched 1-moved 2-released 3
        unsigned char data[2];
        data[0] = payload[0]; //encoder num
        data[1] = (payload[1] > 0 && payload[1] < 30) ? 1 : 0xFF; //1 clockwise, 255 counter clockwise value
        int EventType = payload[2]; //state
        // LOG_INFO("%d %d %d", payload[0], payload[1], payload[2]);
        eventHandle(0xB0, EventType, &data[0]);
    } else if(cmd.cmd == static_cast<unsigned char>(PushFaker::FakerCmd::System)) {
        if(payload[0] == 1) {
            LOG_INFO("Device connected");
        } else if(payload[0] == 2) {
            LOG_INFO("Device disconnected");
        }
    }
}

}