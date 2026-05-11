// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <string>
#include <cstdio>

namespace Common {

bool fileExists(const std::string& path) {
    FILE* file = std::fopen(path.c_str(), "r");
    if (file) {
        std::fclose(file);
        return true;
    }
    return false;
}

bool deleteFile(const std::string& path) {
    return std::remove(path.c_str()) == 0;
}

}