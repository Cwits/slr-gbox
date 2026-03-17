// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/push/PadLayoutWidget.h"

#include "push/PushPainter.h"
#include "push/PushContext.h"
#include "push/helper.h"

#include "ui/push/primitives/StaticLabel.h"
#include "ui/push/primitives/Pointer.h"
#include "ui/push/PushUIContext.h"
#include "ui/push/PadNoteLayout.h"
#include "ui/PushUIDefines.h"

#include "logger.h"
#include <string>
#include <cmath>

namespace PushUI {
static const std::string_view CHROMATIC_TEXT = "Chromatic";
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

#define CLBS(x) \
    X(DisplayBottom1, toggleChromatic) \
    X(DisplayBottom2, setRootNote) \
    X(DisplayBottom3, setRootNote) \
    X(DisplayBottom5, setRootNote) \
    X(DisplayBottom6, setRootNote) \
    X(DisplayBottom7, setRootNote) \
    X(DisplayTop2, setRootNote) \
    X(DisplayTop3, setRootNote) \
    X(DisplayTop4, setRootNote) \
    X(DisplayTop5, setRootNote) \
    X(DisplayTop6, setRootNote) \
    X(DisplayTop7, setRootNote) \
    X(DisplayTop8, setRootNote) \
    X(Down, pointerTestInc)     \
    X(Up, pointerTestDec)

const PushLib::ButtonCallbackMap<PadLayoutWidget> PadLayoutWidget::_buttonsCallback = {
#define X(btn, clb) {PushLib::Button::btn, &PadLayoutWidget::clb},
    CLBS(X)
#undef X
};

const std::vector<PushLib::ButtonColor> _buttonColors = {
#define X(btn, clb) { PushLib::Button::btn, PushLib::LedAnimation(), 127 },
    CLBS(X)
#undef X
};

PadLayoutWidget::PadLayoutWidget(PushLib::Widget *parent, PushUIContext * const puictx) :
    Widget(parent),
    _pUIctx(puictx)
{
    position(0, 0);
    size(PushLib::DISPLAY_WIDTH, PushLib::DISPLAY_HEIGHT);
    _padNotes = std::make_unique<PadNoteLayout>(puictx);
    puictx->pctx()->setPadsLayout(_padNotes.get());
    
    _pointerPos = 1;
    
    PushLib::Font f = PushLib::Font_11x18;
    int x = 0;
    int y = 0;
    for(int i=0; i<static_cast<int>(Scales::LAST); ++i) {
        const std::string_view &text = PadNoteLayout::scaleName(static_cast<Scales>(i));

        std::unique_ptr<StaticLabel> lbl = std::make_unique<StaticLabel>(this, text);
        intToXY(x, y, i+1);
        x += 20;
        lbl->position(x, y);
        lbl->size(200, 18);
        lbl->font(f);
        _scaleNames.push_back(std::move(lbl));
    }

    _pointer = std::make_unique<Pointer>(this);
    intToXY(x, y, _pointerPos);
    _pointer->position(x, y); //0x7fff6c09d890
    _pointer->size(7, 10);
}

PadLayoutWidget::~PadLayoutWidget() {

}

void PadLayoutWidget::paint(PushLib::Painter &painter) {
    // if(dirty()) {
        // clearDirty();
        // painter.clearRegion(bounds());

        // //do whatever you need to do(e.g. fill background)
        // int xl = 20;
        // int yl = PushUIDefines::DefaultMargin;
        // PushLib::Font f = PushUIDefines::DefaultFont;
        // PushLib::Color c = PushLib::COLORS::White;

        // // painter.clear();
        
        // xl = PushUIDefines::DisplayButtonWidth + (PushUIDefines::DisplayButtonWidth/2);;
        // painter.writeChar(xl, yl, 'C', f, c);
        // xl += PushUIDefines::DisplayButtonWidth;
        // painter.writeChar(xl, yl, 'D', f, c);
        // xl += PushUIDefines::DisplayButtonWidth;
        // painter.writeChar(xl, yl, 'E', f, c);
        // xl += PushUIDefines::DisplayButtonWidth;
        // painter.writeChar(xl, yl, 'F', f, c);
        // xl += PushUIDefines::DisplayButtonWidth;
        // painter.writeChar(xl, yl, 'G', f, c);
        // xl += PushUIDefines::DisplayButtonWidth;
        // painter.writeChar(xl, yl, 'A', f, c);
        // xl += PushUIDefines::DisplayButtonWidth;
        // painter.writeChar(xl, yl, 'B', f, c);

        // xl = 10;
        // yl = PushLib::DISPLAY_HEIGHT-PushUIDefines::DefaultMargin-f.FontWidth;
        // bool chromatic = _padNotes->chromatic();
        // painter.writeString(xl, yl, std::string("Chromatic"), f, chromatic ? PushLib::COLORS::Red : c);
        // xl = PushUIDefines::DisplayButtonWidth + (PushUIDefines::DisplayButtonWidth/2);;
        // painter.writeString(xl, yl, std::string("C#"), f, c);
        // xl += PushUIDefines::DisplayButtonWidth;
        // painter.writeString(xl, yl, std::string("D#"), f, c);
        // xl += (PushUIDefines::DisplayButtonWidth*2);
        // painter.writeString(xl, yl, std::string("F#"), f, c);
        // xl += PushUIDefines::DisplayButtonWidth;
        // painter.writeString(xl, yl, std::string("G#"), f, c);
        // xl += PushUIDefines::DisplayButtonWidth;
        // painter.writeString(xl, yl, std::string("A#"), f, c);

    // }
    painter.clear();
    painter.writeString(50, 70, std::string("ARIERGJWEPOFwpifwoiejfwpaWERF"), PushLib::Font_11x18, PushLib::COLORS::White);
}

bool PadLayoutWidget::handleButton(PushLib::ButtonEvent &ev) {
    const auto res = _buttonsCallback.find(ev.button);
    if(res != _buttonsCallback.end()) {
        const auto cb = res->second;
        return (this->*cb)(ev);
    }

    return _pUIctx->tryHandleButtonDefault(ev);
}

bool PadLayoutWidget::handleEncoder(PushLib::EncoderEvent &ev) {
    if(ev.encoder < PushLib::Encoder::Encoder1 && ev.encoder > PushLib::Encoder::Encoder8) {
        return _pUIctx->tryHandleEncoderDefault(ev);
    }

    if(ev.type != PushLib::EncoderEventType::Moved) return false;

    if( !PushHelper::slowdown(5) ) return false;

    if(ev.encoder == PushLib::Encoder::Encoder8) {
        //handle layout switch
    } else {
        //scale switch
        // _redrawPointer = true;
        // if(ev.delta > 0) {
        //     if(_pointerPos < static_cast<int>(Scales::LAST)) _pointerPos++;
        // } else if(ev.delta < 0) {
        //     if(_pointerPos > 1) _pointerPos--;
        // }
        
        // Scales sk = static_cast<Scales>(_pointerPos-1);
        // _padNotes->setScale(sk);
    }
    
    return true;
}

std::vector<PushLib::ButtonColor> PadLayoutWidget::buttonsColors() { return _buttonColors; }

bool PadLayoutWidget::toggleChromatic(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;

    bool chroma = _padNotes->chromatic();
    _padNotes->setChromatic(!chroma);

    // if(chroma) _chroma->color(PushLib::COLORS::Red);
    // else _chroma->color(PushLib::COLORS::White);

    // _chroma->markDirty();
    // markDirty();

    return true;
}

bool PadLayoutWidget::setRootNote(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;

    if(ev.button == PushLib::Button::DisplayTop2) _padNotes->setRootNote(36);
    else if(ev.button == PushLib::Button::DisplayBottom2) _padNotes->setRootNote(37);
    else if(ev.button == PushLib::Button::DisplayTop3) _padNotes->setRootNote(38);
    else if(ev.button == PushLib::Button::DisplayBottom3) _padNotes->setRootNote(39);
    else if(ev.button == PushLib::Button::DisplayTop4) _padNotes->setRootNote(40);
    else if(ev.button == PushLib::Button::DisplayTop5) _padNotes->setRootNote(41);
    else if(ev.button == PushLib::Button::DisplayBottom5) _padNotes->setRootNote(42);
    else if(ev.button == PushLib::Button::DisplayTop6) _padNotes->setRootNote(43);
    else if(ev.button == PushLib::Button::DisplayBottom6) _padNotes->setRootNote(44);
    else if(ev.button == PushLib::Button::DisplayTop7) _padNotes->setRootNote(45);
    else if(ev.button == PushLib::Button::DisplayBottom7) _padNotes->setRootNote(46);
    else if(ev.button == PushLib::Button::DisplayTop8) _padNotes->setRootNote(47);

    return true;
}

bool PadLayoutWidget::pointerTestInc(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;

    int x,y;
    _pointerPos++;
    intToXY(x, y, _pointerPos);
    _pointer->position(x, y);
    return true;
}

bool PadLayoutWidget::pointerTestDec(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;

    int x,y;
    if(_pointerPos > 1) _pointerPos--;
    intToXY(x, y, _pointerPos);
    _pointer->position(x, y);
    return true;
}


}