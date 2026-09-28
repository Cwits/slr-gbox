// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "display2/primitives/BaseWidget.h"

namespace Display {

struct Context;
struct Button;

struct Grid : public BaseWidget {
    Grid(BaseWidget *parent, Context *ctx);
    ~Grid();

    private:
    Context * const _ctx;
    std::unique_ptr<Button> _btnTphide;
    std::unique_ptr<Button> _btnBphide;
};

}