// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once 

#include "modules/Track/Track.h"

#include "snapshots/AudioUnitView.h"
#include "defines.h"

#include <string>

namespace slr {

class TrackView : public AudioUnitView {
    public:
    TrackView(Track * tr);
    ~TrackView();
    
    const bool record() const { return _record; }
    const RecordSource recordSource() const { return _recordSource; }
    
    // void setArm(ID parameterId, float value);

    void update() override;

    private:
    bool _record;
    RecordSource _recordSource;

    const Track * _track;
};


}