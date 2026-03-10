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
 
struct GridWidget : public PushLib::Widget {
    GridWidget(PushLib::Widget *parent, PushUIContext * const puictx);
    ~GridWidget();

    PushLib::BoundingBox invalidate() override; //return BoundingBox of area that has to be redrawn
    void paint(PushLib::Painter &painter) override;

    bool handleButton(PushLib::ButtonEvent &ev) override;
    bool handleEncoder(PushLib::EncoderEvent &ev) override;

    std::vector<PushLib::ButtonColor> buttonsColors() override;
    private:
    PushUIContext * const _pUIctx;
    
    int RectX;      //test purpose
    int RectY;      //test purpose
    int RectColor;  //test purpose

    static const PushLib::ButtonCallbackMap<GridWidget> _buttonsCallback;

    bool upBtnClb(PushLib::ButtonEvent &ev);
    bool downBtnClb(PushLib::ButtonEvent &ev);
    bool leftBtnClb(PushLib::ButtonEvent &ev);
    bool rightBtnClb(PushLib::ButtonEvent &ev);
};

}