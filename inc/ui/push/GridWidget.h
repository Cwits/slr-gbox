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

class PushUIContext;
 
struct GridWidget : public PushLib::Widget {
    GridWidget(PushLib::Widget *parent, PushUIContext * const puictx);
    ~GridWidget();

    void paint(PushLib::Painter &painter) override;

    bool handleButton(PushLib::ButtonEvent &ev) override;
    bool handleEncoder(PushLib::EncoderEvent &ev) override;

    std::vector<PushLib::ButtonColor> buttonsColors() override;
    
    private:
    PushUIContext * const _pUIctx;

    // BUTTONS
    static const PushLib::ButtonCallbackMap<GridWidget> _buttonsCallback;

    bool upBtnClb(PushLib::ButtonEvent &ev);
    bool downBtnClb(PushLib::ButtonEvent &ev);
    bool leftBtnClb(PushLib::ButtonEvent &ev);
    bool rightBtnClb(PushLib::ButtonEvent &ev);
};

}