// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/push/UnitWidget.h"

#include "push/PushContext.h"
#include "push/PushPainter.h"
#include "push/helper.h"

#include "ui/push/PushUIContext.h"
#include "ui/push/primitives/Canvas.h"
#include "ui/push/primitives/Rectangle.h"

#include <cmath>

namespace PushUI {

#define CLBS(x) \
    X(Left, leftCallback) \
    X(Right, rightCallback) \

const PushLib::ButtonCallbackMap<UnitWidget> UnitWidget::_buttonsCallback = {
#define X(btn, clb) {PushLib::Button::btn, &UnitWidget::clb},
    CLBS(X)
#undef X
};

const std::vector<PushLib::ButtonColor> _buttonColors = {
#define X(btn, clb) { PushLib::Button::btn, PushLib::LedAnimation(), 127 },
    CLBS(X)
#undef X
};

UnitWidget::UnitWidget(PushLib::Widget *parent, PushUIContext * const puictx) :
    Widget(parent),
    _pUIctx(puictx)
{
    position(0, 0);
    size(PushLib::DISPLAY_WIDTH, PushLib::DISPLAY_HEIGHT);

    _canvas = std::make_unique<Canvas>(this, PushLib::DISPLAY_WIDTH-160, PushLib::DISPLAY_HEIGHT-40);
    _canvas->position(80, 20);

    _rect = std::make_unique<Rectangle>(this, true);
    _rect->position(70, 70);
    _rect->size(20, 20);
    _rect->color(PushLib::rgb(255, 0, 255));
}

UnitWidget::~UnitWidget() {

}

void UnitWidget::paint(PushLib::Painter &painter) {
    painter.clear();
    painter.writeString(400, 50, std::string("Module"), PushLib::Font_11x18, PushLib::COLORS::White);
}

bool UnitWidget::handleButton(PushLib::ButtonEvent &ev) {
    const auto res = _buttonsCallback.find(ev.button);
    if(res != _buttonsCallback.end()) {
        const auto cb = res->second;
        return (this->*cb)(ev);
    }

    return _pUIctx->tryHandleButtonDefault(ev);
}

bool UnitWidget::handleEncoder(PushLib::EncoderEvent &ev) {
    return false;
}

std::vector<PushLib::ButtonColor> UnitWidget::buttonsColors() {  return _buttonColors; }

bool UnitWidget::leftCallback(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;

    PushLib::Vec2 v = _rect->position();
    _rect->position(v._x-10, v._y);
    return true;
}

bool UnitWidget::rightCallback(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;

    PushLib::Vec2 v = _rect->position();
    _rect->position(v._x+10, v._y);
    return true;
}


}