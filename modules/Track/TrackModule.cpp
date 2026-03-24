// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modules/Track/TrackModule.h"

#include "core/ModuleManager.h"
#include "core/primitives/ClipContainer.h"

#include "ui/display/primitives/UnitUIBase.h"

#include "modules/Track/Track.h"
#include "modules/Track/TrackUI.h"
#include "modules/Track/TrackView.h"

#include <memory>

std::unique_ptr<slr::AudioUnit> createTrackRT(const slr::ClipContainer * initContainer) { 
    return std::make_unique<slr::Track>(initContainer);
}

std::unique_ptr<slr::AudioUnitView> createTrackView(slr::AudioUnit * track) {
    return std::make_unique<slr::TrackView>(static_cast<slr::Track*>(track));
}

UI::UnitUIBase * createTrackUI(slr::AudioUnitView * track, UI::UIContext * uictx) {
    return new UI::TrackUI(track, uictx);
}

const std::string_view _trackName = "Track";

const slr::Module TrackModule {
    ._name = &_trackName,
    ._type = slr::ModuleType::Basic,
    .createRT = createTrackRT,
    .createView = createTrackView,
    .createUI = createTrackUI
};


