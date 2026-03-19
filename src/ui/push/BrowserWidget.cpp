// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/push/BrowserWidget.h"

#include "push/PushContext.h"
#include "push/PushPainter.h"

#include "ui/push/PushUIContext.h"

namespace PushUI {

BrowserWidget::BrowserWidget(PushLib::Widget *parent, PushUIContext * const puictx) :
    Widget(parent),
    _pUIctx(puictx)
{
    position(0, 0);
    size(PushLib::DISPLAY_WIDTH, PushLib::DISPLAY_HEIGHT);
}

BrowserWidget::~BrowserWidget() {

}

void BrowserWidget::paint(PushLib::Painter &painter) {
    painter.clear();
    painter.writeString(400, 50, std::string("Browser"), PushLib::Font_11x18, PushLib::COLORS::White);
}

bool BrowserWidget::handleButton(PushLib::ButtonEvent &ev) {
    return _pUIctx->tryHandleButtonDefault(ev);
}

bool BrowserWidget::handleEncoder(PushLib::EncoderEvent &ev) {
    return false;
}


}