// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

namespace slr {

inline bool floatToBool(float value) {
    if(value >= .5) return true;
    else return false;
}

inline float boolToFloat(bool value) {
    if(value) return 1.0f;
    else return 0.0f;
}

}