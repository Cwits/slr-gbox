// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/MidiEvent.h"
#include "push/PushLib.h"

#include <vector>

namespace PushHelper {

inline slr::MidiEvent buttonColorToEvent(PushLib::ButtonColor &clr) {
    slr::MidiEvent ret;

    ret.type = slr::MidiEventType::CC;
    ret.note = static_cast<uint8_t>(clr.btn);
    ret.velocity = static_cast<uint8_t>(clr.color);
    ret.channel = ( static_cast<uint8_t>(clr.anim.type) | static_cast<uint8_t>(clr.anim.duration) );
    ret.offset = 0;
    return ret;
}

template<typename T>
inline std::vector<PushLib::ButtonColor> buttonColorsFromMap(const PushLib::ButtonCallbackMap<T> &map) {
    std::vector<PushLib::ButtonColor> ret;
    std::size_t size = map.size();
    ret.reserve(size);

    for(const auto &p : map) {
        PushLib::ButtonColor clr;
        clr.btn = p.first;
        clr.color = 127;
        clr.anim = PushLib::LedAnimation();
        ret.push_back(clr);
    }

    return ret;
}

inline bool isBtnPressed(const PushLib::ButtonEvent &ev) {
    return ev.type == PushLib::ButtonEventType::Pressed;
}

inline bool slowdown(char max) {
    static char slowdown = 0;
    slowdown++;
    if(slowdown < max) return false;
    slowdown = 0;
    return true;
}

}