// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/defaultStyles.h"
#include "ui/display/defaultColors.h"
#include "ui/display/layoutSizes.h"

namespace UI {

lv_style_t workspace;
lv_style_t gridLine;
lv_style_t playheadStyle;
lv_style_t loopMarkersStyle;
lv_style_t loopFillStyle;
lv_style_t browserElementStyle;
lv_style_t buttonDefaultStyle;
lv_style_t loopHandleStyle;

void initDefaultStyles() {
    lv_style_init(&workspace);
    lv_style_set_margin_all(&workspace, 0);
    lv_style_set_pad_all(&workspace, 0);
    lv_style_set_radius(&workspace, 0);
    lv_style_set_border_width(&workspace, 0);
    // lv_style_remove_prop(&style, LV_STYLE_BG_COLOR);

    lv_style_init(&buttonDefaultStyle);
    lv_style_set_radius(&buttonDefaultStyle, 5);
    lv_style_set_bg_color(&buttonDefaultStyle, GRAY_COLOR);

    lv_style_init(&gridLine);
    lv_style_set_line_width(&gridLine, 2);
    lv_style_set_line_color(&gridLine, lv_color_hex(0xffffff));
    lv_style_set_line_rounded(&gridLine, true);
    lv_style_set_margin_all(&gridLine, 0);
    lv_style_set_pad_all(&gridLine, 0);
    lv_style_set_radius(&gridLine, 0);
    lv_style_set_border_width(&gridLine, 0);

    lv_style_init(&playheadStyle);
    lv_style_copy(&playheadStyle, &gridLine);
    lv_style_set_line_color(&playheadStyle, lv_color_hex(0x00ff00));
    lv_style_set_line_rounded(&playheadStyle, true);

    lv_style_init(&loopMarkersStyle);    
    lv_style_copy(&loopMarkersStyle, &gridLine);
    lv_style_set_line_color(&loopMarkersStyle, lv_color_hex(0xffff00));
    lv_style_set_line_rounded(&loopMarkersStyle, true);
    lv_style_set_opa(&loopFillStyle, LV_OPA_70);
    
    lv_style_init(&loopFillStyle);    
    lv_style_copy(&loopFillStyle, &workspace);
    lv_style_set_bg_color(&loopFillStyle, lv_color_hex(0xffff00));
    lv_style_set_opa(&loopFillStyle, LV_OPA_30);
    
    lv_style_init(&browserElementStyle);
    lv_style_copy(&browserElementStyle, &workspace);
    lv_style_set_border_width(&browserElementStyle, 1);
    lv_style_set_border_color(&browserElementStyle, lv_color_hex(0x000000));
    lv_style_set_radius(&browserElementStyle, 5);

    lv_style_init(&loopHandleStyle);    
    lv_style_copy(&loopHandleStyle, &workspace);
    lv_style_set_bg_color(&loopHandleStyle, lv_color_hex(0xffff00));
    lv_style_set_opa(&loopHandleStyle, LV_OPA_70);
    lv_style_set_radius(&loopHandleStyle, 5);
}

}