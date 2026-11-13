// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lvgl.h"


namespace UI {

inline const lv_font_t * pickFont(int pix) {
    const lv_font_t * ret;
    if(pix <= 8) ret = &lv_font_montserrat_8;
    else if(pix > 8 && pix <= 10) ret = &lv_font_montserrat_10;
    else if(pix > 10 && pix <= 12) ret = &lv_font_montserrat_12;
    else if(pix > 12 && pix <= 14) ret = &lv_font_montserrat_14;
    else if(pix > 14 && pix <= 16) ret = &lv_font_montserrat_16;
    else if(pix > 16 && pix <= 18) ret = &lv_font_montserrat_18;
    else if(pix > 18 && pix <= 20) ret = &lv_font_montserrat_20;
    else if(pix > 20 && pix <= 22) ret = &lv_font_montserrat_22;
    else if(pix > 22 && pix <= 24) ret = &lv_font_montserrat_24;
    else if(pix > 24 && pix <= 26) ret = &lv_font_montserrat_26;
    else if(pix > 26 && pix <= 28) ret = &lv_font_montserrat_28;
    else if(pix > 28 && pix <= 30) ret = &lv_font_montserrat_30;
    else if(pix > 30 && pix <= 32) ret = &lv_font_montserrat_32;
    else if(pix > 32 && pix <= 34) ret = &lv_font_montserrat_34;
    else if(pix > 34 && pix <= 36) ret = &lv_font_montserrat_36;
    else if(pix > 36 && pix <= 38) ret = &lv_font_montserrat_38;
    else if(pix > 38 && pix <= 40) ret = &lv_font_montserrat_40;
    else if(pix > 40 && pix <= 42) ret = &lv_font_montserrat_42;
    else if(pix > 42 && pix <= 44) ret = &lv_font_montserrat_44;
    else if(pix > 44 && pix <= 46) ret = &lv_font_montserrat_46;
    else if(pix > 46) ret = &lv_font_montserrat_48;
    return ret;
}

}