// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "modules/Track/TrackPushUI.h"

#include "modules/Track/TrackView.h"

#include "snapshots/AudioUnitView.h"

#include "ui/push/PushUIContext.h"

namespace PushUI {

TrackPushUI::TrackPushUI(slr::AudioUnitView * track, PushUIContext * uictx)
    : UnitUIBase(track, uictx),
    _track(static_cast<slr::TrackView*>(track))
{

}

TrackPushUI::~TrackPushUI() {

}

bool TrackPushUI::create(PushUIContext * ctx) {
    _gridUI.reset();
    _unitUI.reset();

    _gridUI = std::make_unique<TrackGridUI>(ctx->gridWidget(), this);
    _unitUI = std::make_unique<TrackUnitUI>(ctx->unitWidget(), this);
    return true;
}

bool TrackPushUI::destroy(PushUIContext * ctx) {
    _gridUI.reset();
    _unitUI.reset();
    return true;
} 


TrackGridUI::TrackGridUI(PushLib::Widget *parent, TrackPushUI * parentUI) 
    : DefaultGridUI(parent, parentUI)
{
}

TrackGridUI::~TrackGridUI() {

}

// void TrackGridUI::pollUIUpdate() override {
// }

void TrackGridUI::paint(PushLib::Painter &painter) {
    DefaultGridUI::paint(painter);
}

TrackUnitUI::TrackUnitUI(PushLib::Widget *parent, TrackPushUI * parentUI) 
    : DefaultUnitUI(parent, parentUI)
{

}

TrackUnitUI::~TrackUnitUI() {

}

// void TrackUnitUI::pollUIUpdate() override {
// }

void TrackUnitUI::paint(PushLib::Painter &painter) {
    DefaultUnitUI::paint(painter);
}

}