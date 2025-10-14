// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/ControlEngine.h"
#include "core/Events.h"
#include "core/AudioBufferManager.h"
#include <gtest/gtest.h>
#include <iostream>
#include <cstring>
#include <condition_variable>
#include <mutex>

TEST(ControlEngine, spamEvents) {
    using namespace slr;
    // ControlEngine::init();
    // std::condition_variable cd;

    // ControlEngine::instance()->regme([&cd](int cnt) {
    //     ASSERT_EQ(cnt, 120);
    //     std::cout << "event count: " << (int)cnt << std::endl;
    //     cd.notify_one();
    // });

    // for(int i=0; i<100; ++i) {
    //     auto e = std::make_unique<Events::ParameterChange>();
    //     e->parameterId = 15;
    //     e->value = 3.14f;    
    //     EmitEvent(std::move(e));
    // }

    // for(int i=0; i<20; ++i) {
    //     auto e = std::make_unique<Events::TransportChange>();
    //     e->state = rand() % 1;    
    //     EmitEvent(std::move(e));
    // }

    // std::mutex lock;
    // std::unique_lock<std::mutex> l(lock);
    // cd.wait(l);
    // ControlEngine::shutdown();
}



int main(int argc, char* argv[]) {
    // loguru::init(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}