// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/AudioBufferManager.h"
#include "defines.h"
#include "slr_config.h"
#include <gtest/gtest.h>
#include <iostream>
#include <cstring>
#include <condition_variable>
#include <mutex>

#define BUFFERSIZE 64

TEST(AudioBufferManager, AcquireAndAccessRegular) {
    using namespace slr;
    // AudioBufferManager::init(BUFFERSIZE, DEFAULT_BUFFER_CHANNELS);    
    // AudioBuffer * test = AudioBufferManager::acquireRegular();
    // bool res = true;
    // for(uint8_t i=0; i<test->channels(); ++i) {
    //     sample_t * b = (*test)[i];

    //     for(frame_t f=0; f<test->bufferSize(); ++f) {
    //         b[f] = 0.5f;
    //     }
        
    //     for(frame_t f=0; f<test->bufferSize(); ++f) {
    //         if(b[f] != 0.5f) res = false;
    //     }
    // }
    // ASSERT_TRUE(res);
   
    // AudioBufferManager::releaseRegular(test);
    // AudioBufferManager::shutdown();
}

TEST(AudioBufferManager, ExpandRegular) {
    using namespace slr;
    // AudioBufferManager::init(BUFFERSIZE, DEFAULT_BUFFER_CHANNELS);

    // int buffers = DEFAULT_BUFFER_COUNT + (DEFAULT_BUFFER_COUNT/4);
    // std::vector<AudioBuffer*> test;
    // test.reserve(buffers);
    // test.clear();

    // for(int i=0; i<buffers; ++i) {
    //     AudioBuffer * buf = AudioBufferManager::acquireRegular();
    //     test.push_back(buf);
    // }

    // AudioBuffer * last = test.at(test.size()-1);
    // sample_t * ptr = (*last)[0];
    // sample_t data = 0.5f;
        
    // for(frame_t f=0; f<last->bufferSize(); ++f) {
    //     ptr[f] = data;    
    // }

    // for(frame_t f=0; f<last->bufferSize(); ++f) {
    //     ASSERT_DOUBLE_EQ(data, ptr[f]);
    // }

    // ASSERT_EQ(AudioBufferManager::freeRegularCount(), 16);
    // ASSERT_EQ(AudioBufferManager::totalRegularCount(), 96);

    // for(int i=0; i<buffers; ++i) {
    //     AudioBufferManager::releaseRegular(test.at(i));
    // }

    // ASSERT_EQ(AudioBufferManager::freeRegularCount(), 96);

    // AudioBufferManager::shutdown();
}


TEST(AudioBufferManager, ExpandRecord) {
    using namespace slr;
    // AudioBufferManager::init(BUFFERSIZE, DEFAULT_BUFFER_CHANNELS);

    // int buffers = DEFAULT_RECORD_BUFFER_COUNT + (DEFAULT_RECORD_BUFFER_COUNT/2);
    // std::vector<AudioBuffer*> test;
    // test.reserve(buffers);
    // test.clear();

    // for(int i=0; i<buffers; ++i) {        
    //     AudioBuffer * buf = AudioBufferManager::acquireRecord();
    //     test.push_back(buf);
    // }

    // AudioBuffer * last = test.at(test.size()-1);
    // sample_t * ptr = (*last)[0];
    // sample_t data = 0.5f;
        
    // for(frame_t f=0; f<last->bufferSize(); ++f) {
    //     ptr[f] = data;    
    // }

    // for(frame_t f=0; f<last->bufferSize(); ++f) {
    //     ASSERT_DOUBLE_EQ(data, ptr[f]);
    // }

    // // ASSERT_EQ(AudioBufferManager::freeRecordCount(), 0);
    // ASSERT_EQ(AudioBufferManager::totalRecordCount(), buffers);

    // for(int i=0; i<buffers; ++i) {
    //     AudioBufferManager::releaseRecord(test.at(i));
    // }

    // // ASSERT_EQ(AudioBufferManager::freeRecordCount(), buffers);

    // AudioBufferManager::shutdown();
}

int main(int argc, char* argv[]) {
    // loguru::init(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}