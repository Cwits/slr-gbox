// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modules/Track/TrackView.h"
#include "modules/Track/Track.h"
#include "core/utility/helper.h"


namespace slr {


TrackView::TrackView(Track * tr) : 
    AudioUnitView(tr), 
    _track(tr) 
{
    _record = tr->record();
    _recordSource = tr->recordSource();
}

TrackView::~TrackView() {

}

void TrackView::update() {
    AudioUnitView::update();
    _record = _track->record();
    _recordSource = _track->recordSource();

    //update File container
}
    
}