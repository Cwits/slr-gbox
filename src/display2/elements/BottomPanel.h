// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "display2/primitives/BaseWidget.h"

#include <memory>

namespace Display {

struct Context;

struct BottomPanel : public BaseWidget {
    BottomPanel(BaseWidget * parent, Context *ctx);
    ~BottomPanel();

    private:
    Context * const _ctx;
};


}