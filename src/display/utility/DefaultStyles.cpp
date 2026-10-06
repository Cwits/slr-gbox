// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display/utility/DefaultStyles.h"
#include "display/utility/DefaultColors.h"
#include "display/utility/layoutSizes.h"

namespace UI {
namespace Style {

lv_style_t borderless;
lv_style_t workspace;
lv_style_t gridLine;
lv_style_t GridControl;
lv_style_t GridFile;
lv_style_t playheadStyle;
lv_style_t loopMarkersStyle;
lv_style_t loopFillStyle;
lv_style_t browserElementStyle;
lv_style_t buttonDefaultStyle;
lv_style_t loopHandleStyle;
lv_style_t PopupDefault;
lv_style_t Panels;
lv_style_t Label;
lv_style_t Text;
lv_style_t DropDown;

void initDefaultStyles() {
    lv_style_init(&borderless);
    lv_style_set_margin_all(&borderless, 0);
    lv_style_set_pad_all(&borderless, 0);
    lv_style_set_radius(&borderless, 0);
    lv_style_set_border_width(&borderless, 0);

    lv_style_init(&workspace);
    lv_style_copy(&workspace, &borderless);
    lv_style_set_bg_color(&workspace, Colors::WorkspaceBackground);
    // lv_style_remove_prop(&style, LV_STYLE_BG_COLOR);

    lv_style_init(&buttonDefaultStyle);
    lv_style_set_radius(&buttonDefaultStyle, 5);
    lv_style_set_bg_color(&buttonDefaultStyle, Colors::ButtonReleased);

    lv_style_init(&gridLine);
    lv_style_set_line_width(&gridLine, 2);
    lv_style_set_line_color(&gridLine, Colors::GridLine);
    lv_style_set_line_rounded(&gridLine, true);
    lv_style_set_margin_all(&gridLine, 0);
    lv_style_set_pad_all(&gridLine, 0);
    lv_style_set_radius(&gridLine, 0);
    lv_style_set_border_width(&gridLine, 0);

    lv_style_init(&playheadStyle);
    lv_style_copy(&playheadStyle, &gridLine);
    lv_style_set_line_color(&playheadStyle, Colors::Green);
    lv_style_set_line_rounded(&playheadStyle, true);

    lv_style_init(&loopMarkersStyle);    
    lv_style_copy(&loopMarkersStyle, &gridLine);
    lv_style_set_line_color(&loopMarkersStyle, Colors::LoopLine);
    lv_style_set_line_rounded(&loopMarkersStyle, true);
    lv_style_set_opa(&loopFillStyle, LV_OPA_70);
    
    lv_style_init(&loopFillStyle);    
    lv_style_copy(&loopFillStyle, &borderless);
    lv_style_set_bg_color(&loopFillStyle, Colors::LoopLine);
    lv_style_set_opa(&loopFillStyle, LV_OPA_30);
    
    lv_style_init(&browserElementStyle);
    lv_style_copy(&browserElementStyle, &borderless);
    lv_style_set_border_width(&browserElementStyle, 1);
    lv_style_set_border_color(&browserElementStyle, Colors::Black);
    lv_style_set_radius(&browserElementStyle, 5);

    lv_style_init(&loopHandleStyle);    
    lv_style_copy(&loopHandleStyle, &borderless);
    lv_style_set_bg_color(&loopHandleStyle, Colors::LoopLine);
    lv_style_set_opa(&loopHandleStyle, LV_OPA_70);
    lv_style_set_radius(&loopHandleStyle, 5);

    lv_style_init(&PopupDefault);
    lv_style_copy(&PopupDefault, &borderless);
    lv_style_set_radius(&PopupDefault, 5);
    lv_style_set_bg_color(&PopupDefault, Colors::PopupBackground);

    lv_style_init(&Panels);
    lv_style_copy(&Panels, &borderless);
    lv_style_set_bg_color(&Panels, Colors::PanelsBackground);

    lv_style_init(&Label);
    lv_style_copy(&Label, &borderless);
    lv_style_set_bg_opa(&Label, LV_OPA_0);
    
    lv_style_init(&Text);
    lv_style_set_text_color(&Text, Colors::Text);
    lv_style_set_text_font(&Text, &DEFAULT_FONT);

    lv_style_init(&GridControl);
    lv_style_copy(&GridControl, &borderless);

    lv_style_init(&GridFile);
    lv_style_copy(&GridFile, &borderless);

    lv_style_init(&DropDown);
    lv_style_copy(&DropDown, &borderless);
    lv_style_set_bg_color(&DropDown, Colors::DropDownBackground);
}

}

}