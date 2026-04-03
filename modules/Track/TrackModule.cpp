// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modules/Track/TrackModule.h"

#include "core/ModuleManager.h"
#include "core/primitives/ClipContainer.h"

#include "ui/display/primitives/UnitUIBase.h"
#include "ui/push/primitives/UnitUIBase.h"

#include "modules/Track/Track.h"
#include "modules/Track/TrackUI.h"
#include "modules/Track/TrackView.h"
#include "modules/Track/TrackPushUI.h"

#include <memory>

std::unique_ptr<slr::AudioUnit> createTrackRT(const slr::ClipContainer * initContainer) { 
    return std::make_unique<slr::Track>(initContainer);
}

std::unique_ptr<slr::AudioUnitView> createTrackView(slr::AudioUnit * track) {
    return std::make_unique<slr::TrackView>(static_cast<slr::Track*>(track));
}

std::unique_ptr<UI::UnitUIBase> createTrackUI(slr::AudioUnitView * track, UI::UIContext * uictx) {
    return std::make_unique<UI::TrackUI>(track, uictx);
}

std::unique_ptr<PushUI::UnitUIBase> createTrackPushUI(slr::AudioUnitView * track, PushUI::PushUIContext * uictx) {
    return std::make_unique<PushUI::TrackPushUI>(track, uictx);
}

const std::string_view _trackName = "Track";

const slr::Module TrackModule {
    ._name = &_trackName,
    ._type = slr::ModuleType::Basic,
    .createRT = createTrackRT,
    .createView = createTrackView,
    .createUI = createTrackUI,
    .createPushUI = createTrackPushUI
};


