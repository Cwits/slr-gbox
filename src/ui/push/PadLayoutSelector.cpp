// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/push/PadLayoutSelector.h"

#include "push/PushPainter.h"

#include "ui/push/PadNoteLayout.h"
#include "ui/PushUIDefines.h"

#include <string>

namespace PushUI {

PadLayoutSelector::PadLayoutSelector(PushLib::Widget *parent, PushLib::PushContext * const pctx) :
    Widget(parent, pctx)
{
    _padNotes = std::make_unique<PadNoteLayout>(pctx);
    pctx->setPadsLayout(_padNotes.get());
    _pointer = std::make_unique<Pointer>(this, pctx);
    addChild(_pointer.get());
}

PadLayoutSelector::~PadLayoutSelector() {

}

PushLib::BoundingBox PadLayoutSelector::invalidate() {
    PushLib::BoundingBox box;
    return box; 
}

void PadLayoutSelector::paint(PushLib::Painter &painter) {
    int x = 20;
    int y = PushUIDefines::DefaultMargin;
    PushLib::FontDef f = PushUIDefines::DefaultFont;
    PushLib::Color c = PushLib::COLORS::White;

    painter.clear();
    
    x = PushUIDefines::DisplayButtonWidth + (PushUIDefines::DisplayButtonWidth/2);;
    painter.writeChar(x, y, 'C', f, c);
    x += PushUIDefines::DisplayButtonWidth;
    painter.writeChar(x, y, 'D', f, c);
    x += PushUIDefines::DisplayButtonWidth;
    painter.writeChar(x, y, 'E', f, c);
    x += PushUIDefines::DisplayButtonWidth;
    painter.writeChar(x, y, 'F', f, c);
    x += PushUIDefines::DisplayButtonWidth;
    painter.writeChar(x, y, 'G', f, c);
    x += PushUIDefines::DisplayButtonWidth;
    painter.writeChar(x, y, 'A', f, c);
    x += PushUIDefines::DisplayButtonWidth;
    painter.writeChar(x, y, 'B', f, c);

    x = 10;
    y = PushLib::DISPLAY_HEIGHT-PushUIDefines::DefaultMargin-f.FontWidth;
    painter.writeString(x, y, std::string("Chromatic"), f, c);
    x = PushUIDefines::DisplayButtonWidth + (PushUIDefines::DisplayButtonWidth/2);;
    painter.writeString(x, y, std::string("C#"), f, c);
    x += PushUIDefines::DisplayButtonWidth;
    painter.writeString(x, y, std::string("D#"), f, c);
    x += (PushUIDefines::DisplayButtonWidth*2);
    painter.writeString(x, y, std::string("F#"), f, c);
    x += PushUIDefines::DisplayButtonWidth;
    painter.writeString(x, y, std::string("G#"), f, c);
    x += PushUIDefines::DisplayButtonWidth;
    painter.writeString(x, y, std::string("A#"), f, c);

    
    f = PushLib::Font_11x18;
    x = 20;
    y = 22;
    for(int i=0; i<static_cast<int>(Scales::LAST); ++i) {
        const std::string_view &text = PadNoteLayout::scaleName(static_cast<Scales>(i));
    
        painter.writeString(x, y, text, f, PushLib::COLORS::White);
        y += (f.FontHeight + 7);
        if(y+f.FontHeight+5 >= PushLib::DISPLAY_HEIGHT) {
            x += 200;
            y = 22;
        }
    }
}

bool PadLayoutSelector::handleButton(PushLib::ButtonEvent &ev) {
    return false;
}

bool PadLayoutSelector::handleEncoder(PushLib::EncoderEvent &ev) {
    if(ev.delta > 0) {

    } else if(ev.delta < 0) {

    }
    return false;
}



}