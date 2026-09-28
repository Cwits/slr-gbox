// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "display2/utility/DefaultStyle.h"
#include "display2/utility/DefaultSizes.h"
#include "display2/utility/DefaultColors.h"

namespace Display {

namespace DefaultStyle {

lv_style_t Borderless;
lv_style_t ButtonStyle;
lv_style_t Workspace;
// lv_style_t ButtonLabelStyle;
lv_style_t LabelStyle;

const lv_font_t *Font;

lv_style_t gridLine;
lv_style_t playheadStyle;
lv_style_t loopMarkersStyle;
lv_style_t loopFillStyle;
lv_style_t loopHandleStyle;
lv_style_t browserElementStyle;

void initDefaultStyle() {
    DefaultColors::initDefaultColors();

    Font = &lv_font_montserrat_30;

    lv_style_init(&Borderless);
    lv_style_set_margin_all(&Borderless, 0);
    lv_style_set_pad_all(&Borderless, 0);
    lv_style_set_border_width(&Borderless, 0);
    lv_style_set_radius(&Borderless, 0);

    lv_style_init(&Workspace);
    lv_style_copy(&Workspace, &Borderless);
    lv_style_set_radius(&Workspace, 0);

    lv_style_init(&ButtonStyle);
    // lv_style_set_radius(&ButtonStyle, 5);
    lv_style_set_bg_color(&ButtonStyle, DefaultColors::Button);
    lv_style_set_text_font(&ButtonStyle, Font);
    lv_style_set_size(&ButtonStyle, DefaultSize::Button, DefaultSize::Button);

    lv_style_init(&LabelStyle);
    lv_style_copy(&LabelStyle, &Borderless);
    lv_style_set_bg_opa(&LabelStyle, LV_OPA_TRANSP);
    lv_style_set_text_font(&LabelStyle, Font);
    lv_style_set_width(&LabelStyle, LV_SIZE_CONTENT);
    lv_style_set_height(&LabelStyle, lv_font_get_line_height(Font));
    lv_style_set_text_color(&LabelStyle, DefaultColors::TextColor);

    
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
    lv_style_copy(&loopFillStyle, &Borderless);
    lv_style_set_bg_color(&loopFillStyle, lv_color_hex(0xffff00));
    lv_style_set_opa(&loopFillStyle, LV_OPA_30);
    lv_style_set_radius(&loopFillStyle, 0);
    
    lv_style_init(&loopHandleStyle);    
    lv_style_copy(&loopHandleStyle, &Borderless);
    lv_style_set_bg_color(&loopHandleStyle, lv_color_hex(0xffff00));
    lv_style_set_opa(&loopHandleStyle, LV_OPA_70);
    lv_style_set_radius(&loopHandleStyle, 5);
    
    lv_style_init(&browserElementStyle);
    lv_style_copy(&browserElementStyle, &Borderless);
    lv_style_set_border_width(&browserElementStyle, 1);
    lv_style_set_border_color(&browserElementStyle, lv_color_hex(0x000000));
    lv_style_set_radius(&browserElementStyle, 5);
    
}

}

}