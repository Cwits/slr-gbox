// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/push/PadLayoutSelector.h"

#include "push/PushPainter.h"
#include "push/PushContext.h"
#include "push/helper.h"

#include "ui/push/PushUIContext.h"
#include "ui/push/PadNoteLayout.h"
#include "ui/PushUIDefines.h"

#include "logger.h"
#include <string>
#include <cmath>

namespace PushUI {

/* 
    1. 20 22        220 22
    2. 20 47        220 47
    3. 20 72        220 72
    4. 20 97        220 97
    5. 20 122       220 122
*/
void intToXY(int &x, int &y, int i) {
    switch(i) {
        case(1): { x = 5; y = 22; } break;
        case(2): { x = 5; y = 47; } break;
        case(3): { x = 5; y = 72; } break;
        case(4): { x = 5; y = 97; } break;
        case(5): { x = 5; y = 122; } break;
        case(6): { x = 205; y = 22; } break;
        case(7): { x = 205; y = 47; } break;
        case(8): { x = 205; y = 72; } break;
        case(9): { x = 205; y = 97; } break;
        case(10): { x = 205; y = 122; } break;
        case(11): { x = 405; y = 22; } break;
        case(12): { x = 405; y = 47; } break;
        case(13): { x = 405; y = 72; } break;
        case(14): { x = 405; y = 97; } break;
        case(15): { x = 405; y = 122; } break;
    }
}

PadLayoutSelector::PadLayoutSelector(PushLib::Widget *parent, PushUIContext * const puictx) :
    Widget(parent),
    _pUIctx(puictx)
{
    _padNotes = std::make_unique<PadNoteLayout>(puictx);
    puictx->pctx()->setPadsLayout(_padNotes.get());
    
    _pointerX = 5;
    _pointerY = 22;
    _pointerPos = 1;
    _pointerOldPos = 1;
}

PadLayoutSelector::~PadLayoutSelector() {

}

PushLib::BoundingBox PadLayoutSelector::invalidate() {
    PushLib::BoundingBox box;
    if(_redrawPointer) {
        //return bounding box between old pointer position and new
        int xold, yold;
        int xnew, ynew;
        intToXY(xold, yold, _pointerOldPos);
        intToXY(xnew, ynew, _pointerPos);

        box.x = std::min(xold, xnew);
        box.y = std::min(yold, ynew);
        box.w = std::abs(xnew-xold) + PushLib::Font_11x18.FontWidth;
        box.h = std::abs(ynew-yold) + PushLib::Font_11x18.FontHeight;
    } else {
        box.x = 0;
        box.y = 0;
        box.w = PushLib::DISPLAY_WIDTH;
        box.h = PushLib::DISPLAY_HEIGHT;
    }

    //return full display bounding box?
    return box; 
}

void PadLayoutSelector::paint(PushLib::Painter &painter) {
    if(_redrawPointer) {
        int xold;
        int yold;
        intToXY(xold, yold, _pointerOldPos);
        painter.clearRegion(xold, yold, 11, 18);
        intToXY(xold, yold, _pointerPos);
        painter.writeChar(xold, yold, '>', PushLib::Font_11x18, PushLib::COLORS::White);
        _pointerOldPos = _pointerPos;
        _redrawPointer = false;
        return;
    }

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

    painter.writeChar(_pointerX, _pointerY, '>', PushLib::Font_11x18, PushLib::COLORS::White);
}

bool PadLayoutSelector::handleButton(PushLib::ButtonEvent &ev) {
    if( (ev.button >= PushLib::Button::DisplayBottom1 && ev.button <= PushLib::Button::DisplayBottom8) ||
        (ev.button >= PushLib::Button::DisplayTop1 && ev.button <= PushLib::Button::DisplayTop8) )
    {
        LOG_INFO("Handle me layout pads");
        return true;
    } else {
        return _pUIctx->tryHandleButtonDefault(ev);
    }
}

bool PadLayoutSelector::handleEncoder(PushLib::EncoderEvent &ev) {
    if(ev.encoder < PushLib::Encoder::Encoder1 && ev.encoder > PushLib::Encoder::Encoder8) {
        return _pUIctx->tryHandleEncoderDefault(ev);
    }

    if(ev.type != PushLib::EncoderEventType::Moved) return false;

    if( !PushHelper::slowdown(5) ) return false;

    if(ev.encoder == PushLib::Encoder::Encoder8) {
        //handle layout switch
    } else {
        //scale switch
        _redrawPointer = true;
        // PushLib::FontDef f = PushLib::Font_11x18;
        if(ev.delta > 0) {
            if(_pointerPos < static_cast<int>(Scales::LAST)) _pointerPos++;
        } else if(ev.delta < 0) {
            if(_pointerPos > 1) _pointerPos--;
        }
        
        //get scale
        //apply scale
    
        Scales sk = static_cast<Scales>(_pointerPos-1);
        _padNotes->setScale(sk);
        _pUIctx->pctx()->updatePadsColors();
    }
    
    return true;
}

}