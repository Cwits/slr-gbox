// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "modules/Sampler/SamplerPushUI.h"
#include "modules/Sampler/SamplerView.h"

#include "snapshots/AudioUnitView.h"

#include "ui/push/PushUIContext.h"

namespace PushUI {

SamplerGridUI::SamplerGridUI(PushLib::Widget *parent, SamplerPushUI * parentUI) 
    : DefaultGridUI(parent, parentUI)
{

}

SamplerGridUI::~SamplerGridUI() {

}

void SamplerGridUI::paint(PushLib::Painter &painter) {
    DefaultGridUI::paint(painter);
}


SamplerUnitUI::SamplerUnitUI(PushLib::Widget *parent, SamplerPushUI * parentUI) 
    : DefaultUnitUI(parent, parentUI)
{

}

SamplerUnitUI::~SamplerUnitUI() {

}

void SamplerUnitUI::paint(PushLib::Painter &painter) {
    DefaultUnitUI::paint(painter);
}


SamplerPushUI::SamplerPushUI(slr::AudioUnitView * sampler, PushUIContext * uictx) 
    : UnitUIBase(sampler, uictx),
    _sampler(static_cast<slr::SamplerView*>(sampler))
{

}

SamplerPushUI::~SamplerPushUI() {

}

bool SamplerPushUI::create(PushUIContext * ctx) {
    _gridUI.reset();
    _unitUI.reset();

    _gridUI = std::make_unique<SamplerGridUI>(ctx->gridWidget(), this);
    _unitUI = std::make_unique<SamplerUnitUI>(ctx->unitWidget(), this);
    return true;
}

bool SamplerPushUI::destroy(PushUIContext * ctx) {
    _gridUI.reset();
    _unitUI.reset();


    return true;
}


}