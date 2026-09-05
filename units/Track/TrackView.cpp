// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/Track/TrackView.h"
#include "units/Track/Track.h"
#include "units/Track/TrackUnit.h"

#include "core/utility/helper.h"

#include "core/actions/Actions.h"
#include "units/Track/TrackActions.h"


namespace slr {


TrackView::TrackView(Track * tr) : 
    AudioUnitView(tr), 
    _track(tr) 
{
    _name = "Track " + std::to_string(_uniqueId);
    _record = tr->record();
    _recordSource = tr->recordSource();
}

TrackView::~TrackView() {

}

void TrackView::update() {
    // AudioUnitView::update();
    _record = _track->record();
    _recordSource = _track->recordSource();
    incrementVersion();
    //update File container
}
    
std::string TrackView::unitType() { 
    std::string ret = "internal:";
    ret += *TrackDescriptor._name;
    return ret;
}

nlohmann::ordered_json TrackView::saveUnit() {
    nlohmann::ordered_json ret;

    ret = AudioUnitView::saveUnit();

    ret["Record Arm"] = record();
    ret["Record Source"] = static_cast<int>(recordSource());

    return ret;
}

void TrackView::loadUnit(const nlohmann::json &data) {
    AudioUnitView::loadUnit(data);


    auto act = std::make_unique<slr::Actions::RecordArm>();
    act->targetId = id();
    act->recordState = boolToFloat(data["Record Arm"].get<bool>());
    act->recordSource = static_cast<slr::RecordSource>(data["Record Source"].get<int>());
    slr::EmitAction(std::move(act));
}

}