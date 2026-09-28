// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "display2/elements/BottomPanel.h"

#include "display2/primitives/Button.h"
#include "display2/primitives/Label.h"

#include "display2/utility/DefaultStyle.h"
#include "display2/utility/DefaultSizes.h"
#include "display2/utility/DefaultColors.h"

namespace Display {

BottomPanel::BottomPanel(BaseWidget * parent, Context *ctx) : 
    BaseWidget(parent, true), 
    _ctx(ctx)
{
    setPos(0, parent->height() - DefaultSize::BottomPanelHeight);
    setSize(parent->width(), DefaultSize::BottomPanelHeight);

    lv_obj_add_style(_lvhost, &DefaultStyle::Borderless, 0);
    lv_obj_set_style_bg_color(_lvhost, DefaultColors::Panels, LV_PART_MAIN);
}

BottomPanel::~BottomPanel() {

}

}