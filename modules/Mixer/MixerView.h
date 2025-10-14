// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "snapshots/AudioUnitView.h"

namespace slr {
class Mixer;

struct MixerView : public AudioUnitView {
    MixerView(Mixer * const mix);
    ~MixerView();

    void update() override;

    private:
    const Mixer * const _mixer;
};

}