// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "push/PushPadLayout.h"
#include "push/PushLeds.h"
#include "push/PushMidi.h"

#include "logger.h"

#include <vector>
#include <string>

namespace PushLib {

const Layout _originalLayout = 
{
    36, 37, 38, 39, 40, 41, 42, 43,
    44, 45, 46, 47, 48, 49, 50, 51,
    52, 53, 54, 55, 56, 57, 58, 59,
    60, 61, 62, 63, 64, 65, 66, 67,
    68, 69, 70, 71, 72, 73, 74, 75, 
    76, 77, 78, 79, 80, 81, 82, 83,
    84, 85, 86, 87, 88, 89, 90, 91,
    92, 93, 94, 95, 96, 97, 98, 99
};

const Layout _layout4 = 
{
    36, 37, 38, 39, 40, 41, 42, 43,
    41, 42, 43, 44, 45, 46, 47, 48,
    46, 47, 48, 49, 50, 51, 52, 53,
    51, 52, 53, 54, 55, 56, 57, 58,
    56, 57, 58, 59, 60, 61, 62, 63,
    61, 62, 63, 64, 65, 66, 67, 68,
    66, 67, 68, 69, 70, 71, 72, 73,
    71, 72, 73, 74, 75, 76, 77, 78
};

const std::vector<Layout> _layouts = {
    _originalLayout,
    _layout4
};

const std::vector<Scale> _scales =
{
    {1, 3, 5, 6, 8, 10, 12},    //Scales::Major
    {1, 3, 5, 6, 8, 9,  12},    //Scales::MajorHarmonic
    {1, 3, 4, 6, 8, 9,  11},    //Scales::Minor
    {1, 3, 4, 6, 8, 10, 11},    //Scales::MinorHarmonic
    {1, 3, 4, 6, 8, 9,  11},    //Scales::MinorMelodic
    {1, 3, 4, 6, 8, 10, 11},    //Scales::Dorian
    {1, 2, 4, 6, 8, 9,  11},    //Scales::Phrygian
    {1, 3, 5, 7, 8, 10, 12},    //Scales::Lydian
    {1, 3, 5, 6, 8, 10, 11},    //Scales::Mixolydian
    {1, 3, 4, 6, 8, 9,  11},    //Scales::Aeolian
    {1, 2, 4, 6, 7, 9,  11},    //Scales::Locrian
    {1, 3, 5, 8, 10},           //Scales::MajorPentatonic
    {1, 3, 4, 5, 8, 10},        //Scales::MajorBlues
    {1, 4, 6, 8, 11},           //Scales::MinorPentatonic
    {1, 4, 6, 7, 8, 11},        //Scales::Blues
};

const std::vector<std::string_view> _scalesTexts = {
    "Natural Major",
    "Harmonic Major",
    "Natural Minor",
    "Harmonic Minor",
    "Melodic Minor",
    "Dorian",
    "Phrygian",
    "Lydian",
    "Mixolydian",
    "Aeolian",
    "Locrian",
    "Major Pentatonic",
    "Major Blues Pentanonic",
    "Minor Pentatonic",
    "Blues"
};


PushPadLayout::PushPadLayout(PushMidi &midi, PushLeds &leds) :
    _leds(leds),
    _midi(midi),
    _chromatic(true),
    _rootNote(36), //C2
    _octave(12),
    _scale(Scales::NaturalMajor),
    _layout(LayoutStyle::Layout4),
    _rootPadColor(99),
    _offScalePadColor(90),
    _inScalePadColor(44),
    _padPressedColor(26)
{

}

PushPadLayout::~PushPadLayout() {

}

const Layout & PushPadLayoutlayout(const LayoutStyle style) {
    return _layouts[static_cast<unsigned char>(style)];
}

const unsigned char PushPadLayout::getNote(const unsigned char &pad) {
    const unsigned char currentLayoutNum = static_cast<unsigned char>(_layout.load(std::memory_order_acquire));
    const Layout &currentLayout = _layouts[currentLayoutNum];

    const unsigned char padFromLayout = currentLayout[pad-36];

    const bool isChromatic = _chromatic.load(std::memory_order_acquire);
    unsigned char note = 0;
    
    if(isChromatic) {
        note = ((padFromLayout-36) % 12) + 1;
    } else {
        //in scale        
        const unsigned char currentScale = static_cast<unsigned char>(_scale.load(std::memory_order_acquire));
        const Scale &sk = _scales[currentScale];

        unsigned char noteNum = ((padFromLayout-36) % static_cast<unsigned char>(sk.size()));
        note = sk[noteNum];
    }

    return note;
}

void PushPadLayout::renderPads() {
    
    std::vector<slr::MidiEvent> batch;
    batch.reserve(64);
    
    for(int i=36; i<99; ++i) {
        const unsigned char note = getNote(i);

        slr::MidiEvent ev;
        ev.type = slr::MidiEventType::NoteOn;
        ev.note = i;
        ev.channel = 0;
        if(inScale(note)) {
            if(note == 1) ev.velocity = _rootPadColor;
            else ev.velocity = _inScalePadColor;
        } else {
            ev.velocity = _offScalePadColor;
        }
        batch.push_back(ev);
    }

    _midi.sendBatch(batch);
}

void PushPadLayout::renderPad(const unsigned char &pad, const slr::MidiEventType &type) {
    const LayoutStyle &currentLayoutStyle = _layout.load(std::memory_order_acquire);

    switch(currentLayoutStyle) {
        case(LayoutStyle::Default): {
            //always one color
            slr::MidiEvent ev;
            ev.channel = 0;
            ev.type = type;
            ev.note = pad;
            if(type == slr::MidiEventType::NoteOn) {
                // _leds.setLedColor(pad, LedAnimation(), _padPressedColor);
                ev.velocity = _padPressedColor;
            } else if(type == slr::MidiEventType::NoteOff) {
                const unsigned char note = getNote(pad);
                if(inScale(note)) {
                    if(note == 1) ev.velocity = _rootPadColor; /*_leds.setLedColor(pad, LedAnimation(), _rootPadColor);*/
                    else ev.velocity = _inScalePadColor; /*_leds.setLedColor(pad, LedAnimation(), _inScalePadColor);*/
                } else {
                   ev.velocity = _offScalePadColor; /* _leds.setLedColor(pad, LedAnimation(), _offScalePadColor);*/
                }
            }
            _midi.sendSingle(ev);
        } break;
        case(LayoutStyle::Layout4): {
            const unsigned char currentLayoutNum = static_cast<unsigned char>(_layout.load(std::memory_order_acquire));
            const Layout &currentLayout = _layouts[currentLayoutNum];
            const unsigned char realPad = currentLayout[pad-36];
            std::vector<unsigned char> pads;
            pads.reserve(2);
            for(int i=0; i<currentLayout.size(); ++i) {
                if(currentLayout.at(i) == realPad) {
                    pads.push_back(i+36);
                }
            }

            for(int i=0; i<pads.size(); ++i) {
                slr::MidiEvent ev;
                const unsigned char pd = pads.at(i);
                ev.channel = 0;
                ev.type = type;
                ev.note = pd;
                if(type == slr::MidiEventType::NoteOn) {
                    ev.velocity = _padPressedColor;
                } else {
                    const unsigned char note = getNote(pd);
                    if(inScale(note)) {
                        if(note == 1) ev.velocity = _rootPadColor; /*_leds.setLedColor(pd, LedAnimation(), _rootPadColor);*/
                        else ev.velocity = _inScalePadColor; /*_leds.setLedColor(pd, LedAnimation(), _inScalePadColor);*/
                    } else {
                        ev.velocity = _offScalePadColor; /*_leds.setLedColor(pd, LedAnimation(), _offScalePadColor);*/
                    }
                }
                _midi.sendSingle(ev);
            }
        } break;
    }
    
}

bool PushPadLayout::inScale(const unsigned char &note) {
    const unsigned char currentScale = static_cast<unsigned char>(_scale.load(std::memory_order_acquire));
    const Scale &sk = _scales[currentScale];
    std::size_t size = sk.size();

    bool ret = false;
    for(std::size_t i=0; i<size; ++i) {
        if(sk[i] == note) {
            ret = true;
            break;
        }
    }

    return ret;
}


}