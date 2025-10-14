// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lvgl.h"
// #include "ui/display/primitives/BaseWidget.h"
#include "ui/display/primitives/View.h"

namespace UI {
class UIContext;

class ModuleView : public View {
    public:
    ModuleView(BaseWidget * parent, UIContext * const uictx);
    ~ModuleView();

    void update() override;

    lv_obj_t * _lb;
    private:
};

}