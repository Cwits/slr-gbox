// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "ui/display/primitives/BaseWidget.h"

#include <functional>

namespace UI {

struct Checkbox : public BaseWidget {
    Checkbox(BaseWidget * parent);
    ~Checkbox();

    void setCallback(std::function<void(bool)> fn);
    void setValue(bool checked);

    private:
    lv_obj_t * _checked;
    bool _state;

    void updateCheck();

    std::function<void(bool)> _callback;

    bool handleTap(GestLib::TapGesture &tap) override;
};

}