// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

namespace Display {

struct RootWindow;

struct Context {
    Context(RootWindow *root);
    ~Context();

    private:
    RootWindow * const _root;
};

}