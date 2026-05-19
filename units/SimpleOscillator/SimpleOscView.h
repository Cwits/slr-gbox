// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "units/SimpleOscillator/SimpleOsc.h"

#include "snapshots/AudioUnitView.h"
#include "defines.h"

#include <string>

namespace slr {

class SimpleOscView : public AudioUnitView {
    public:
    SimpleOscView(SimpleOsc * osc);
    ~SimpleOscView();
    
    void update() override;

    std::string unitType() override;

    nlohmann::ordered_json saveUnit() override;

    private:

    const SimpleOsc * _osc;
};


}