// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/push/ModuleWidget.h"

#include "push/PushContext.h"
#include "push/PushPainter.h"

#include "ui/push/PushUIContext.h"

namespace PushUI {

ModuleWidget::ModuleWidget(PushLib::Widget *parent, PushUIContext * const puictx) :
    Widget(parent),
    _pUIctx(puictx)
{

}

ModuleWidget::~ModuleWidget() {

}

PushLib::BoundingBox ModuleWidget::invalidate() {
    return PushLib::BoundingBox();
}

void ModuleWidget::paint(PushLib::Painter &painter) {

    painter.clear();
    painter.writeString(400, 50, std::string("Module"), PushLib::Font_11x18, PushLib::COLORS::White);
}

bool ModuleWidget::handleButton(PushLib::ButtonEvent &ev) {
    return _pUIctx->tryHandleButtonDefault(ev);
}

bool ModuleWidget::handleEncoder(PushLib::EncoderEvent &ev) {
    return false;
}


}