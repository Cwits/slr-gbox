// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/Widget.h"

#include <memory>
#include <vector>

namespace PushLib {
    class Painter;
}

namespace PushUI {

class StaticLabel;
class Pointer;
class PadNoteLayout;
class PushUIContext;

struct PadLayoutWidget : public PushLib::Widget {
    PadLayoutWidget(PushLib::Widget *parent, PushUIContext * const puictx);
    ~PadLayoutWidget();

    void paint(PushLib::Painter &p) override;

    bool handleButton(PushLib::ButtonEvent &ev) override;
    bool handleEncoder(PushLib::EncoderEvent &ev) override;

    std::vector<PushLib::ButtonColor> buttonsColors() override;

    private:
    PushUIContext * const _pUIctx;
    
    std::unique_ptr<PadNoteLayout> _padNotes;

    static const PushLib::ButtonCallbackMap<PadLayoutWidget> _buttonsCallback;

    int _pointerPos;
    int _pointerOldPos;
    
    bool _redrawPointer;

    std::unique_ptr<Pointer> _pointer;
    std::vector<std::unique_ptr<StaticLabel>> _scaleNames;
    std::unique_ptr<StaticLabel> _lblChromatic;

    bool toggleChromatic(PushLib::ButtonEvent &ev);
    bool setRootNote(PushLib::ButtonEvent &ev);
    bool pointerTestInc(PushLib::ButtonEvent &ev);
    bool pointerTestDec(PushLib::ButtonEvent &ev);
};  

}