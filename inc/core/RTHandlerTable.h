/* This file is generated automatically, do not edit manually */
// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "modules/Track/Track.h"
#include "core/Project.h"
#include "core/Timeline.h"
#include "core/primitives/AudioUnit.h"
#include "core/RtEngine.h"

#include "core/FlatEvents.h"
#include "common/Status.h"
namespace slr {

namespace RTHandlers {

using RTHandler = Common::Status(*)(const FlatEvents::FlatControl&, FlatEvents::FlatResponse&);
extern "C" {
constexpr RTHandler RTTable[] = {
	&Project::modifyClipItem,
	&Timeline::requestPlayhead,
	&Timeline::changeTimelineState,
	&Timeline::toggleLoop,
	&Timeline::setBpmTimeSig,
	&Timeline::setLoopPosition,
	&RtEngine::updateMidiMaps,
	&Project::swapPlan,
	&AudioUnit::setParameter,
	&AudioUnit::toggleMidiThru,
	&AudioUnit::toggleOmniHwInput,
	&AudioUnit::swapContainer,
	&Track::setRecordArm,
	&Track::reinitRecord
};
}

} //namespace RTHandlers

} //namespace slr