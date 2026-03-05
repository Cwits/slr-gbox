// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/push/PadNoteLayout.h"

#include "push/PushContext.h"
#include "push/PushLeds.h"
#include "ui/uiutility.h"

#include "logger.h"

#include <vector>
#include <string>

namespace PushUI {


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

PadNoteLayout::PadNoteLayout(PushLib::PushContext * const pctx) :
    _pctx(pctx),
    _chromatic(true),
    _rootNote(36), //C2
    _octave(0),
    _scale(Scales::NaturalMajor),
    _layout(LayoutStyle::Layout4),
    _rootPadColor(99),
    _offScalePadColor(90),
    _inScalePadColor(44),
    _padPressedColor(26)
{

}

PadNoteLayout::~PadNoteLayout() {

}

const unsigned char PadNoteLayout::getNote(const unsigned char &pad) {
    unsigned char note = getNoteInt(pad);
    unsigned char octave = pad / 12;
    unsigned char recreated = (note-1) + (octave*12) + _octave;

    return recreated;
}

unsigned char PadNoteLayout::getNoteInt(const unsigned char &pad) {
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

void PadNoteLayout::renderPads(PushLib::PushLeds &leds) {
    std::vector<PushLib::Pad> batch;
    batch.reserve(64);
    
    for(int i=36; i<=99; ++i) {
        const unsigned char note = getNoteInt(i);
        PushLib::Pad p;
        p.num = i;
        p.anim = PushLib::LedAnimation();
        
        if(inScale(note)) {
            if(note == 1) p.color = _rootPadColor;
            else p.color = _inScalePadColor;
        } else {
            p.color = _offScalePadColor;
        }
        batch.push_back(p);
    }

    leds.setPadsColor(batch);
}

void PadNoteLayout::renderPad(PushLib::PushLeds &leds, const unsigned char &pad, const slr::MidiEventType &type) {
    const LayoutStyle &currentLayoutStyle = _layout.load(std::memory_order_acquire);

    switch(currentLayoutStyle) {
        case(LayoutStyle::Default): {
            //always one color
            PushLib::Pad p;
            p.num = pad;
            p.anim = PushLib::LedAnimation();
            
            if(type == slr::MidiEventType::NoteOn) {
                p.color = _padPressedColor;
            } else if(type == slr::MidiEventType::NoteOff) {
                const unsigned char note = getNoteInt(pad);
                if(inScale(note)) {
                    if(note == 1) p.color = _rootPadColor; 
                    else p.color = _inScalePadColor; 
                } else {
                   p.color = _offScalePadColor; 
                }
            }
            leds.setPadColor(p);
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
                const unsigned char pd = pads.at(i);
                PushLib::Pad p;
                p.anim = PushLib::LedAnimation();
                p.num = pd;
                if(type == slr::MidiEventType::NoteOn) {
                    p.color = _padPressedColor;
                } else {
                    const unsigned char note = getNoteInt(pd);
                    if(inScale(note)) {
                        if(note == 1) p.color = _rootPadColor;
                        else p.color = _inScalePadColor;
                    } else {
                        p.color = _offScalePadColor;
                    }
                }
                leds.setPadColor(p);
            }
        } break;
    }
    
}

const Layout & PadNoteLayout::layout(const LayoutStyle style) {
    return _layouts[static_cast<unsigned char>(style)];
}

bool PadNoteLayout::inScale(const unsigned char &note) {
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

const std::string_view & PadNoteLayout::scaleName(const Scales scale) {
    if(scale >= Scales::LAST) return UIUtility::ErrorString;

    return _scalesTexts[static_cast<std::size_t>(scale)];
}

void PadNoteLayout::setChromatic(const bool chromatic) { 
    _chromatic.store(chromatic, std::memory_order_relaxed); 
    _pctx->updatePadsColors();    
}


void PadNoteLayout::setColors(unsigned char root, unsigned char inScale, unsigned char offScale, unsigned char pressed) {
    _rootPadColor = root;
    _offScalePadColor = offScale;
    _inScalePadColor = inScale;
    _padPressedColor = pressed;
    _pctx->updatePadsColors();
}

void PadNoteLayout::setScale(const Scales scale) { 
    _scale.store(scale, std::memory_order_relaxed); 
    _pctx->updatePadsColors();
}

void PadNoteLayout::setLayout(const LayoutStyle layout) { 
    _layout.store(layout, std::memory_order_relaxed); 
    _pctx->updatePadsColors();
}


}