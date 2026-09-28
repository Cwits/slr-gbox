// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "display2/primitives/BaseWidget.h"

#include <memory>

namespace Display {

struct Label;
struct Button;
struct Context;

struct TopPanel : public BaseWidget {
    TopPanel(BaseWidget *parent, Context *ctx);
    ~TopPanel();

    private:
    Context * const _ctx;
    std::unique_ptr<Label> _lblProjectName;
    std::unique_ptr<Button> _btnGrid;
};

}