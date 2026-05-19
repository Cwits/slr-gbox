// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "units/SimpleOscillator/SimpleOscPushUI.h"
#include "units/SimpleOscillator/SimpleOscView.h"

#include "snapshots/AudioUnitView.h"

#include "ui/push/PushUIContext.h"

#include <cassert>

namespace PushUI {

SimpleOscPushUI::SimpleOscPushUI(const std::shared_ptr<const slr::AudioUnitView> &osc, PushUIContext * uictx)
    : UnitUIBase(osc, uictx),
    _simpleOsc(std::dynamic_pointer_cast<const slr::SimpleOscView>(osc))
{
    assert(!_simpleOsc.expired() && "Unable to cast pointer");
}

SimpleOscPushUI::~SimpleOscPushUI() {

}

bool SimpleOscPushUI::create(PushUIContext * ctx) {
    _gridUI.reset();
    _unitUI.reset();

    _gridUI = std::make_unique<SimpleOscGridUI>(ctx->gridWidget(), this);
    _unitUI = std::make_unique<SimpleOscUnitUI>(ctx->unitWidget(), this);
    return true;
}

bool SimpleOscPushUI::destroy(PushUIContext * ctx) {
    _gridUI.reset();
    _unitUI.reset();
    return true;
} 


SimpleOscGridUI::SimpleOscGridUI(PushLib::Widget *parent, SimpleOscPushUI * parentUI) 
    : DefaultGridUI(parent, parentUI)
{
}

SimpleOscGridUI::~SimpleOscGridUI() {

}

// void SimpleOscGridUI::pollUIUpdate() override {
// }

void SimpleOscGridUI::paint(PushLib::Painter &painter) {
    DefaultGridUI::paint(painter);
}

SimpleOscUnitUI::SimpleOscUnitUI(PushLib::Widget *parent, SimpleOscPushUI * parentUI) 
    : DefaultUnitUI(parent, parentUI)
{

}

SimpleOscUnitUI::~SimpleOscUnitUI() {

}

// void SimpleOscUnitUI::pollUIUpdate() override {
// }

void SimpleOscUnitUI::paint(PushLib::Painter &painter) {
    DefaultUnitUI::paint(painter);
}

}