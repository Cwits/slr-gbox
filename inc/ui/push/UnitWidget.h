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
class Canvas;
class Rectangle;

struct UnitWidget : public PushLib::Widget {
    UnitWidget(PushLib::Widget *parent, PushUIContext * const puictx);
    ~UnitWidget();

    void paint(PushLib::Painter &p) override;
    
    bool handleButton(PushLib::ButtonEvent &ev) override;
    bool handleEncoder(PushLib::EncoderEvent &ev) override;

    std::vector<PushLib::ButtonColor> buttonsColors() override;

    private:
    PushUIContext * const _pUIctx;

    std::unique_ptr<Canvas> _canvas;
    std::unique_ptr<Rectangle> _rect;
    
    static const PushLib::ButtonCallbackMap<UnitWidget> _buttonsCallback;
    bool leftCallback(PushLib::ButtonEvent &ev);
    bool rightCallback(PushLib::ButtonEvent &ev);
};

}