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

const std::string pathToTestFiles = "/home/portablejoe/slr/tests/files/";
const slr::frame_t testFileSize = 64;

bool file_exists(const std::string& path) {
    FILE* file = std::fopen(path.c_str(), "r");
    if (file) {
        std::fclose(file);
        return true;
    }
    return false;
}

bool delete_file(const std::string& path) {
    return std::remove(path.c_str()) == 0;
}

TEST(AudioEdit, Cut) {
    
}

int main(int argc, char* argv[]) {
    // loguru::init(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}