// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "units/Mixer/MixerPushUI.h"
#include "units/Mixer/MixerView.h"

#include "snapshots/AudioUnitView.h"

#include "push/utility/PushUIContext.h"

#include <cassert>

namespace PushUI {

MixerPushUI::MixerPushUI(const std::shared_ptr<const slr::AudioUnitView> &mix, PushUIContext * uictx)
    : UnitUIBase(mix, uictx),
    _mixer(std::dynamic_pointer_cast<const slr::MixerView>(mix))
{
    assert(!_mixer.expired() && "Unable to convert pointer");
}

MixerPushUI::~MixerPushUI() {

}

bool MixerPushUI::create(PushUIContext * ctx) {
    _gridUI.reset();
    _unitUI.reset();

    _gridUI = std::make_unique<MixerGridUI>(ctx->gridWidget(), this);
    _unitUI = std::make_unique<MixerUnitUI>(ctx->unitWidget(), this);
    return true;
}

bool MixerPushUI::destroy(PushUIContext * ctx) {
    _gridUI.reset();
    _unitUI.reset();
    return true;
} 


MixerGridUI::MixerGridUI(PushLib::Widget *parent, MixerPushUI * parentUI) 
    : DefaultGridUI(parent, parentUI)
{
}

MixerGridUI::~MixerGridUI() {

}

// void MixerGridUI::pollUIUpdate() override {
// }

void MixerGridUI::paint(PushLib::Painter &painter) {
    DefaultGridUI::paint(painter);
}

MixerUnitUI::MixerUnitUI(PushLib::Widget *parent, MixerPushUI * parentUI) 
    : DefaultUnitUI(parent, parentUI)
{

}

MixerUnitUI::~MixerUnitUI() {

}

// void MixerUnitUI::pollUIUpdate() override {
// }

void MixerUnitUI::paint(PushLib::Painter &painter) {
    DefaultUnitUI::paint(painter);
}

}