// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "modules/SimpleOscillator/SimpleOsc.h"

#include "snapshots/AudioUnitView.h"
#include "defines.h"

#include <string>

namespace slr {

class SimpleOscView : public AudioUnitView {
    public:
    SimpleOscView(SimpleOsc * osc);
    ~SimpleOscView();
    
    void update() override;

    private:

    const SimpleOsc * _osc;
};


}