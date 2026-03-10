// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/Widget.h"

#include <memory>

namespace PushLib {
    class Painter;
}

namespace PushUI {

class PadNoteLayout;
class PushUIContext;

struct PadLayoutWidget : public PushLib::Widget {
    PadLayoutWidget(PushLib::Widget *parent, PushUIContext * const puictx);
    ~PadLayoutWidget();

    PushLib::BoundingBox invalidate() override; //return BoundingBox of area that has to be redrawn
    void paint(PushLib::Painter &painter) override;

    bool handleButton(PushLib::ButtonEvent &ev) override;
    bool handleEncoder(PushLib::EncoderEvent &ev) override;

    std::vector<PushLib::ButtonColor> buttonsColors() override { return std::vector<PushLib::ButtonColor>();}

    private:
    PushUIContext * const _pUIctx;
    
    std::unique_ptr<PadNoteLayout> _padNotes;

    static const PushLib::ButtonCallbackMap<PadLayoutWidget> _buttonsCallback;

    int _pointerPos;
    int _pointerOldPos;
    
    bool _redrawPointer;

    bool toggleChromatic(PushLib::ButtonEvent &ev);
    bool setRootNote(PushLib::ButtonEvent &ev);
};

}