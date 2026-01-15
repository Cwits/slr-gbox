// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/BufferManager.h"
#include "slr_config.h"

#include <gtest/gtest.h>
#include <iostream>
#include <cstring>
#include <condition_variable>
#include <mutex>

TEST(BufferManager, AcquireAndAccessRegular) {
    using namespace slr;
}

int main(int argc, char* argv[]) {
    // loguru::init(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}