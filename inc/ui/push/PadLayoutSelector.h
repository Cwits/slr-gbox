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

struct Pointer : public PushLib::Widget {
    Pointer(PushLib::Widget *parent, PushLib::PushContext * const pctx) : Widget(parent, pctx) {}
    ~Pointer() {}

    PushLib::BoundingBox invalidate() override { return PushLib::BoundingBox();}
    void paint(PushLib::Painter &painter) override {}
    bool handleButton(PushLib::ButtonEvent &ev) override { return false; }
    bool handleEncoder(PushLib::EncoderEvent &ev) override { return false; }
};

struct PadLayoutSelector : public PushLib::Widget {
    PadLayoutSelector(PushLib::Widget *parent, PushLib::PushContext * const pctx);
    ~PadLayoutSelector();

    PushLib::BoundingBox invalidate() override; //return BoundingBox of area that has to be redrawn
    void paint(PushLib::Painter &painter) override;

    bool handleButton(PushLib::ButtonEvent &ev) override;
    bool handleEncoder(PushLib::EncoderEvent &ev) override;

    private:
    std::unique_ptr<PadNoteLayout> _padNotes;
    std::unique_ptr<Pointer> _pointer;
};

}