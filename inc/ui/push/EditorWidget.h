// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "push/Widget.h"
#include <memory>

namespace PushLib {
    class Painter;
}

namespace PushUI {

class PushUIContext;

struct EditorWidget : public PushLib::Widget {
    EditorWidget(PushLib::Widget *parent, PushUIContext * const puictx);
    ~EditorWidget();

    void paint(PushLib::Painter &p) override;

    bool handleButton(PushLib::ButtonEvent &ev) override;
    bool handleEncoder(PushLib::EncoderEvent &ev) override;

    std::vector<PushLib::ButtonColor> buttonsColors() override { return std::vector<PushLib::ButtonColor>();}

    private:
    PushUIContext * const _pUIctx;
    
};

}