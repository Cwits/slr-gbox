// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/AudioFile.h"
#include <gtest/gtest.h>
#include <iostream>
#include <cstring>
#include <string>
#include <cstdio>  // std::remove, std::fopen

#include "core/primitives/AudioBuffer.h"
#include "core/utility/basicAudioManipulation.h"
#include "common/FileIO.h"

const std::string pathToTestFiles = "/home/portablejoe/slr/tests/files/";
const slr::frame_t testFileSize = 64;

TEST(AudioEdit, Cut) {
    
}

int main(int argc, char* argv[]) {
    // loguru::init(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}