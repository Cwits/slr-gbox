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

struct ModuleWidget : public PushLib::Widget {
    ModuleWidget(PushLib::Widget *parent, PushUIContext * const puictx);
    ~ModuleWidget();

    PushLib::BoundingBox invalidate() override; //return BoundingBox of area that has to be redrawn
    void paint(PushLib::Painter &painter) override;

    bool handleButton(PushLib::ButtonEvent &ev) override;
    bool handleEncoder(PushLib::EncoderEvent &ev) override;

    std::vector<PushLib::ButtonColor> buttonsColors() override { return std::vector<PushLib::ButtonColor>();}

    private:
    PushUIContext * const _pUIctx;
    
};

}