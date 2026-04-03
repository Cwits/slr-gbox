// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/push/primitives/UnitUIBase.h"

#include "ui/push/primitives/Rectangle.h"
#include "ui/push/primitives/Label.h"

#include "snapshots/AudioUnitView.h"
#include "logger.h"

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
    size(200, 70);
    PushUIContext *ctx = _parentUI->uictx();
    int y = (70 * ctx->unitsCount()) + (5*ctx->unitsCount()) + 13;

    position(5, y);

    _rectangle = std::make_unique<Rectangle>(this, true);
    _rectangle->position(5, 5);
    _rectangle->size(190, 60);
    _rectangle->color(PushLib::rgb(_parentUI->view()->color()));
    
    _rectangle2 = std::make_unique<Rectangle>(this, true);
    _rectangle2->position(10, 10);
    _rectangle2->size(20, 20);
    _rectangle2->color(PushLib::invert(_rectangle->color()));

    _label = std::make_unique<Label>(this, _parentUI->view()->name());
    _label->position(10, 10);
    _label->font(PushLib::Font_7x10);
    _label->color(PushLib::Colors::Black);
}

DefaultGridUI::~DefaultGridUI() {
}


void DefaultGridUI::paint(PushLib::Painter &p) {
    p.filledRectangle(_x, _y, _width, _height, PushLib::Colors::White);
}

void DefaultGridUI::up() {
    auto old = _rectangle2->position();
    _rectangle2->position(old.x(), old.y()-10);
}

void DefaultGridUI::down() {
    auto old = _rectangle2->position();
    _rectangle2->position(old.x(), old.y()+10);
}

void DefaultGridUI::left() {
    auto old = _rectangle2->position();
    _rectangle2->position(old.x()-10, old.y());
}

void DefaultGridUI::right() {
    auto old = _rectangle2->position();
    _rectangle2->position(old.x()+10, old.y());
}


DefaultUnitUI::DefaultUnitUI(PushLib::Widget *parent, UnitUIBase *parentUI)
    : PushLib::Widget(parent),
     _parentUI(parentUI) 
{
    _rectangle = std::make_unique<Rectangle>(this, true);
    _rectangle->position(50, 20);
    _rectangle->size(100, 80);
    _rectangle->color(PushLib::rgb(_parentUI->view()->color()));

    // _label = std::make_unique<Label>(this, _parentUI->view()->name());
    // _label->position(400, 20);
    // _label->size(200, 40);
    // _label->font(PushLib::Font_7x10);
}

DefaultUnitUI::~DefaultUnitUI() {

}

}