// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/PushLib.h"

#include "core/primitives/MidiEvent.h"

#include <atomic>
#include <vector>

namespace PushLib {
    
class PushLeds;
class PushPads;
class PushMidi;

struct PushPadLayout {
    PushPadLayout(PushMidi &midi, PushLeds &leds);
    ~PushPadLayout();

    const unsigned char getNote(const unsigned char &pad);

    void renderPads();
    void renderPad(const unsigned char &pad, const slr::MidiEventType &type);


    bool inScale(const unsigned char &note);

    void octaveUp() { _octave += 12; }
    void octaveDown() { _octave -= 12; }
    const unsigned char octave() const { return _octave; }

    LayoutStyle style() const { return _layout.load(std::memory_order_acquire); }
    const Layout & layout(LayoutStyle style);

    void setChromatic(const bool chromatic) { _chromatic.store(chromatic, std::memory_order_relaxed); }
    const bool chromatic() const { return _chromatic.load(std::memory_order_acquire); }

    void setColors(unsigned char root, unsigned char inScale, unsigned char offScale, unsigned char pressed) {
        _rootPadColor = root;
        _offScalePadColor = offScale;
        _inScalePadColor = inScale;
        _padPressedColor = pressed;
    }

    void setScale(const Scales scale) { _scale.store(scale, std::memory_order_relaxed); }
    void setLayout(const LayoutStyle layout) { _layout.store(layout, std::memory_order_relaxed); }

    private:
    PushLeds &_leds;
    PushMidi &_midi;

    std::atomic<bool> _chromatic;
    std::atomic<unsigned char> _rootNote;
    std::atomic<char> _octave;
    std::atomic<Scales> _scale;
    std::atomic<LayoutStyle> _layout;

    std::atomic<unsigned char> _rootPadColor;
    std::atomic<unsigned char> _offScalePadColor;
    std::atomic<unsigned char> _inScalePadColor;
    std::atomic<unsigned char> _padPressedColor;

    friend class PushPads;
};


}