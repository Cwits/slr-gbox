// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/AudioBuffer.h"
#include "core/SettingsManager.h"
#include "testhelper.h"
#include "slr_config.h"
#include <gtest/gtest.h>
#include <malloc.h>
#include <iostream>
#include <cstring>

TEST(AudioBuffer, BufferAllocation) {
    using namespace slr;
    SettingsManager::init(configPath);
    frame_t blockSize = SettingsManager::getBlockSize();
    std::cout << "Block size: " << blockSize << std::endl;
    AudioBuffer * test = new AudioBuffer(blockSize, DEFAULT_BUFFER_CHANNELS);

    ASSERT_TRUE(test != nullptr);
    ASSERT_TRUE(test->bufferSize() == blockSize);
    ASSERT_TRUE(test->channels() == DEFAULT_BUFFER_CHANNELS);
    if(test->channels() == DEFAULT_BUFFER_CHANNELS) {
        ASSERT_TRUE((*test)[0] != nullptr);
        ASSERT_TRUE((*test)[1] != nullptr);
        bool res = true;
        frame_t size = test->bufferSize();
        for(frame_t i=0; i<size; ++i) {
            if((*test)[0][i] != 0.0f || (*test)[1][i] != 0.0f) {
                res = false;
            }
        }

        ASSERT_TRUE(res);
    } else {
        std::cout << "Lack of testing!" << std::endl;
        FAIL();
    }

    delete test;
}

TEST(AudioBuffer, BufferInjecting) {
    using namespace slr;
    SettingsManager::init(configPath);
    frame_t blockSize = SettingsManager::getBlockSize();

    sample_t ** data = new sample_t*[DEFAULT_BUFFER_CHANNELS];
    data[0] = nullptr; data[1] = nullptr;
    AudioBuffer * buf = new AudioBuffer(data, DEFAULT_BUFFER_CHANNELS, blockSize);
    sample_t * l = new sample_t[blockSize];
    std::memset(l, 0.f, sizeof(sample_t) * blockSize);
    data[0] = l;
    sample_t * r = new sample_t[blockSize];
    std::memset(r, 0.f, sizeof(sample_t) * blockSize);
    data[1] = r;
    
    ASSERT_TRUE(buf != nullptr);
    ASSERT_TRUE(buf->bufferSize() == blockSize);
    ASSERT_TRUE(buf->channels() == DEFAULT_BUFFER_CHANNELS);
    if(buf->channels() == DEFAULT_BUFFER_CHANNELS) {
        ASSERT_TRUE((*buf)[0] != nullptr);
        ASSERT_TRUE((*buf)[1] != nullptr);
        bool res = true;
        frame_t size = buf->bufferSize();
        for(frame_t i=0; i<size; ++i) {
            if((*buf)[0][i] != 0.0f || (*buf)[1][i] != 0.0f) {
                res = false;
            }
        }

        ASSERT_TRUE(res);
    } else {
        //lack of tests!!!
        std::cout << "Lack of testing!" << std::endl;
        FAIL();
    }

    delete [] l;
    delete [] r;
    delete [] data;
    delete buf;
}

TEST(AudioBuffer, WriteAndRead) {
    using namespace slr;
    SettingsManager::init(configPath);
    AudioBuffer * buf = new AudioBuffer(SettingsManager::getBlockSize(), DEFAULT_BUFFER_CHANNELS);

    if(buf->channels() == DEFAULT_BUFFER_CHANNELS) {
        sample_t * l = (*buf)[0];
        sample_t * r = (*buf)[1];

        frame_t size = buf->bufferSize();
        for(frame_t i=0; i<size; ++i) {
            l[i] = 0.4f;
            r[i] = 0.6f;
        }
        
        bool res = true;
        for(frame_t i=0; i<size; ++i) {
            if(l[i] != 0.4f || r[i] != 0.6f) res = false;
        }

        ASSERT_TRUE(res);
    } else {
        std::cout << "Lack of test cases!" << std::endl;
        FAIL();
    }
}

int main(int argc, char* argv[]) {
    // loguru::init(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}