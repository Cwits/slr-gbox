// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/PushPadLayoutBase.h"
#include "push/PushLib.h"
#include <string>
#include <atomic>

namespace PushLib {
    class PushLeds;
}

namespace PushUI {

class PushUIContext;

using Layout = std::vector<unsigned char>;
using Scale = std::vector<unsigned char>;

enum class LayoutStyle {
    Default = 0,
    Layout4 = 1
};

enum class Scales {
    NaturalMajor     = 0, 
    MajorHarmonic    = 1, 
    Minor            = 2, 
    MinorHarmonic    = 3, 
    MinorMelodic     = 4, 
    Dorian           = 5, 
    Phrygian         = 6, 
    Lydian           = 7, 
    Mixolydian       = 8, 
    Aeolian          = 9, 
    Locrian          = 10, 
    MajorPentatonic  = 11, 
    MajorBlues       = 12, 
    MinorPentatonic  = 13, 
    Blues            = 14, 
    LAST
};

struct PadNoteLayout : public PushLib::PushPadLayout {
    PadNoteLayout(PushUIContext * const pctx);
    ~PadNoteLayout();

    const unsigned char getNote(const unsigned char &pad) override;

    void renderPads(PushLib::PushLeds &leds) override;
    void renderPad(PushLib::PushLeds &leds, const unsigned char &pad, const slr::MidiEventType &type) override;

    
    bool inScale(const unsigned char &note);

    void octaveUp() { _octave += 12; }
    void octaveDown() { _octave -= 12; }
    const unsigned char octave() const { return _octave; }

    LayoutStyle style() const { return _layout.load(std::memory_order_acquire); }
    const Layout & layout(LayoutStyle style);

    void setChromatic(const bool chromatic);
    const bool chromatic() const { return _chromatic.load(std::memory_order_acquire); }

    void setColors(unsigned char root, unsigned char inScale, unsigned char offScale, unsigned char pressed);

    void setScale(const Scales scale);
    void setLayout(const LayoutStyle layout);

    static const std::string_view & scaleName(const Scales scale);

    private:
    PushUIContext * const _pctx;

    std::atomic<bool> _chromatic;
    std::atomic<unsigned char> _rootNote;
    std::atomic<char> _octave;
    std::atomic<Scales> _scale;
    std::atomic<LayoutStyle> _layout;

    std::atomic<unsigned char> _rootPadColor;
    std::atomic<unsigned char> _offScalePadColor;
    std::atomic<unsigned char> _inScalePadColor;
    std::atomic<unsigned char> _padPressedColor;

    unsigned char getNoteInt(const unsigned char &pad);
};


}