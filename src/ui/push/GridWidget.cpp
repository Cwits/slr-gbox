// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/push/GridWidget.h"

#include "push/PushContext.h"
#include "push/PushPainter.h"
#include "push/helper.h"

#include "ui/push/PushUIContext.h"

#include "core/Events.h"

#include "logger.h"

namespace PushUI {

const PushLib::ButtonCallbackMap<GridWidget> GridWidget::_buttonsCallback = {
    {PushLib::Button::Up, &GridWidget::upBtnClb},
    {PushLib::Button::Down, &GridWidget::downBtnClb},
    {PushLib::Button::Right, &GridWidget::rightBtnClb},
    {PushLib::Button::Left, &GridWidget::leftBtnClb},
};

GridWidget::GridWidget(PushLib::Widget *parent, PushUIContext * const puictx) :
    Widget(parent),
    _pUIctx(puictx)
{

    RectX = 50;
    RectY = 50;
    RectColor = 50;
}

GridWidget::~GridWidget() {

}

PushLib::BoundingBox GridWidget::invalidate() {
    return PushLib::BoundingBox();
}

void GridWidget::paint(PushLib::Painter &painter) {
    using namespace PushLib;

    painter.clear();
    painter.writeString(400, 50, std::string("Grid"), PushLib::Font_11x18, PushLib::COLORS::White);

    painter.filledRectangle(RectX, 50+RectY, 50, 50, COLORS::Red);
    painter.Rectangle(70+RectX, 50+RectY, 50, 50, 1, COLORS::Green);
    painter.filledRectangle(140+RectX, 50+RectY, 50, 50, rgb(255, 100, 100));
    uint16_t color = 0;
    if(RectColor == 0) color = rgb(255, 0, 0);
    else if(RectColor == 1) color = rgb(0, 255, 0);
    else if(RectColor == 2) color = rgb(0, 0, 255);
    else if(RectColor == 3) color = rgb(255, 255, 0);
    else if(RectColor == 4) color = rgb(255, 0, 255);
    else if(RectColor == 5) color = rgb(0, 255, 255);
    else if(RectColor == 6) color = rgb(100, 100, 100);
    else if(RectColor == 7) color = rgb(255, 255, 255);
    painter.Rectangle(500+RectX, 50+RectY, 50, 50, 1, color);
}

bool GridWidget::handleButton(PushLib::ButtonEvent &ev) {
    const auto res = _buttonsCallback.find(ev.button);
    if(res != _buttonsCallback.end()) {
        const auto cb = res->second;
        return (this->*cb)(ev);
    }

    return _pUIctx->tryHandleButtonDefault(ev);
}

bool GridWidget::handleEncoder(PushLib::EncoderEvent &ev) {
    return false;
}

std::vector<PushLib::ButtonColor> GridWidget::buttonsColors() {
    std::vector<PushLib::ButtonColor> map = PushHelper::buttonColorsFromMap<GridWidget>(GridWidget::_buttonsCallback);
    return map;
}

/* ------------------------  HANDLES  --------------------------- */

bool GridWidget::upBtnClb(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;
    LOG_INFO("Up");

    slr::Events::CreateModule e = {
        .name = "Track"
    };
    slr::EmitEvent(e); 
    
    return false;
}

bool GridWidget::downBtnClb(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;
    LOG_INFO("down");
    return false;
}

bool GridWidget::leftBtnClb(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;
    LOG_INFO("left");
    return false;
}

bool GridWidget::rightBtnClb(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;
    LOG_INFO("right");
    return false;
}

}