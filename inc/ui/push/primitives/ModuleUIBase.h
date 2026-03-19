// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace PushUI {

struct ModuleUIBase {
    ModuleUIBase() {}
    ~ModuleUIBase() {}

    virtual bool create() {return false;}
    virtual bool update() {return false;}
    virtual bool destroy() {return false;}

    
};

}