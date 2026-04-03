// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/push/GridWidget.h"

#include "push/PushContext.h"
#include "push/PushPainter.h"
#include "push/helper.h"

#include "ui/push/PushUIContext.h"
#include "ui/push/primitives/UnitUIBase.h"
#include "ui/push/primitives/Label.h"

#include "core/Events.h"

#include "logger.h"

namespace PushUI {

const PushLib::ButtonCallbackMap<GridWidget> GridWidget::_buttonsCallback = {
    {PushLib::Button::Up, &GridWidget::upBtnClb},
    {PushLib::Button::Down, &GridWidget::downBtnClb},
    {PushLib::Button::Right, &GridWidget::rightBtnClb},
    {PushLib::Button::Left, &GridWidget::leftBtnClb},
    {PushLib::Button::New, &GridWidget::newBtnClb}
};

GridWidget::GridWidget(PushLib::Widget *parent, PushUIContext * const puictx) :
    Widget(parent),
    _pUIctx(puictx)
{
    position(0, 0);
    size(PushLib::DISPLAY_WIDTH, PushLib::DISPLAY_HEIGHT);
    
    _someText1 = std::make_unique<Label>(this, "Some text");
    _someText1->position(1, 1);
    _someText1->font(PushLib::Font_7x10);
    _someText1->color(PushLib::Colors::White);
    _someText2 = std::make_unique<Label>(this, "Some text");
    _someText2->position(200, 1);
    _someText2->font(PushLib::Font_7x10);
    _someText2->color(PushLib::Colors::White);
    _someText3 = std::make_unique<Label>(this, "Some text");
    _someText3->position(400, 1);
    _someText3->font(PushLib::Font_7x10);
    _someText3->color(PushLib::Colors::White);
    _someText4 = std::make_unique<Label>(this, "Some text");
    _someText4->position(600, 1);
    _someText4->font(PushLib::Font_7x10);
    _someText4->color(PushLib::Colors::White);
}

GridWidget::~GridWidget() {

}

void GridWidget::paint(PushLib::Painter &p) {
    p.clearScreen();
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
    // LOG_INFO("Up");
    std::vector<std::unique_ptr<UnitUIBase>> & v = _pUIctx->units();
    for(auto &c : v) {
        c->gridUI()->up();
    }
    return true;
}

bool GridWidget::downBtnClb(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;
    // LOG_INFO("down");
    std::vector<std::unique_ptr<UnitUIBase>> & v = _pUIctx->units();
    for(auto &c : v) {
        c->gridUI()->down();
    }
    return true;
}

bool GridWidget::leftBtnClb(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;
    // LOG_INFO("left");;
    std::vector<std::unique_ptr<UnitUIBase>> & v = _pUIctx->units();
    for(auto &c : v) {
        c->gridUI()->left();
    }
    return false;
}

bool GridWidget::rightBtnClb(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;
    // LOG_INFO("right");;
    std::vector<std::unique_ptr<UnitUIBase>> & v = _pUIctx->units();
    for(auto &c : v) {
        c->gridUI()->right();
    }
    return false;
}

bool GridWidget::newBtnClb(PushLib::ButtonEvent &ev) {
    if(!PushHelper::isBtnPressed(ev)) return false;

    slr::Events::CreateModule e = {
        .name = "Track"
    };
    slr::EmitEvent(e); 
    
    return true;
}

}