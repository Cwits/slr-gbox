// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/push/primitives/UnitUIBase.h"

#include "ui/push/primitives/Rectangle.h"
#include "ui/push/primitives/Label.h"

#include "snapshots/AudioUnitView.h"

namespace PushUI {

UnitUIBase::UnitUIBase(slr::AudioUnitView *view, PushUIContext * const puictx)
    : _view(view),
    _puictx(puictx)
{

}

UnitUIBase::~UnitUIBase() {

}

DefaultGridUI::DefaultGridUI(PushLib::Widget *parent, UnitUIBase *parentUI)
    : PushLib::Widget(parent),
    _parentUI(parentUI) 
{
    size(200, 30);
    PushUIContext *ctx = _parentUI->uictx();
    int y = (30 * ctx->unitsCount())+15;

    position(20, y);

    _rectangle = std::make_unique<Rectangle>(this, true);
    _rectangle->position(20, y);
    _rectangle->size(200, 30);
    _rectangle->color(PushLib::rgb(_parentUI->view()->color()));

    _label = std::make_unique<Label>(this, _parentUI->view()->name());
    _label->position(20, y);
    _label->size(195, 25);
    _label->font(PushLib::Font_7x10);
}

DefaultGridUI::~DefaultGridUI() {

}

void DefaultGridUI::paint(PushLib::Painter &painter) {
    _rectangle->paint(painter);
    _label->paint(painter);
}


DefaultUnitUI::DefaultUnitUI(PushLib::Widget *parent, UnitUIBase *parentUI)
    : PushLib::Widget(parent),
     _parentUI(parentUI) 
{
    _rectangle = std::make_unique<Rectangle>(this, true);
    _rectangle->position(50, 20);
    _rectangle->size(100, 80);
    _rectangle->color(PushLib::rgb(_parentUI->view()->color()));

    _label = std::make_unique<Label>(this, _parentUI->view()->name());
    _label->position(400, 20);
    _label->size(200, 40);
    _label->font(PushLib::Font_7x10);
}

DefaultUnitUI::~DefaultUnitUI() {

}

void DefaultUnitUI::paint(PushLib::Painter &painter) {
    _rectangle->paint(painter);
    _label->paint(painter);
}

}