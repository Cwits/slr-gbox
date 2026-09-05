// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/Track/TrackUnit.h"

#include "core/primitives/ClipContainer.h"

#include "display/primitives/UnitUIBase.h"
#include "push/primitives/UnitUIBase.h"

#include "units/Track/Track.h"
#include "units/Track/TrackUI.h"
#include "units/Track/TrackView.h"
#include "units/Track/TrackPushUI.h"

#include "units/Track/TrackActions.h"
#include "units/Track/Actions/DumpRecordedAudio.h"
#include "units/Track/Actions/RecordArm.h"
#include "units/Track/Actions/ReinitTrackRecord.h"

#include <memory>

std::unique_ptr<slr::AudioUnit> createTrackRT(const slr::ClipContainer * initContainer, const slr::ID forcedId) { 
    return std::make_unique<slr::Track>(initContainer, forcedId);
}

std::shared_ptr<slr::AudioUnitView> createTrackView(slr::AudioUnit * track) {
    return std::make_shared<slr::TrackView>(static_cast<slr::Track*>(track));
}

std::unique_ptr<UI::UnitUIBase> createTrackUI(const std::shared_ptr<const slr::AudioUnitView> &track, UI::UIContext * uictx) {
    return std::make_unique<UI::TrackUI>(track, uictx);
}

std::unique_ptr<PushUI::UnitUIBase> createTrackPushUI(const std::shared_ptr<const slr::AudioUnitView> &track, PushUI::PushUIContext * uictx) {
    return std::make_unique<PushUI::TrackPushUI>(track, uictx);
}

const std::string_view _trackName = "Track";

const slr::UnitDescriptor TrackDescriptor {
    ._name = &_trackName,
    .createRT = createTrackRT,
    .createView = createTrackView,
    .createUI = createTrackUI,
    .createPushUI = createTrackPushUI
};

namespace slr {
void registerTrackActions(std::map<std::type_index, CreatorFn> &map) {
    map[typeid(Actions::DumpRecordedAudio)] = &createDumpRecAudioAction;
    map[typeid(Actions::RecordArm)] = &createRecordArmAction;
    map[typeid(Actions::ReinitTrackRecord)] = &createReinitTrackRecordAction;
}
}