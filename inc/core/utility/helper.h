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

enum class Extention { Audio, Midi, AudioPeak };
inline bool pathHasExtention(Extention e, std::string & path) {
    bool ret = false;
    if(e == Extention::Audio) {
        if(path.substr(path.size()-4).compare(".wav") == 0) {
            ret = true;
        }
    } else if(e == Extention::Midi) {
        if(path.substr(path.size()-4).compare(".mid") == 0) {
            ret = true;
        }
    } else if(e == Extention::AudioPeak) {
        if(path.substr(path.size()-6).compare(".slrpk") == 0) {
            ret = true;
        }
    }

    return ret;
}

}