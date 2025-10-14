// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lvgl.h"

namespace UI {

struct SplashScreen {
    SplashScreen(lv_obj_t * parent);
    ~SplashScreen();

    lv_obj_t * _parent;
    lv_obj_t * _logo;
    lv_obj_t * _host;
};

}