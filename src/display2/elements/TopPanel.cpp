// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display2/elements/TopPanel.h"

#include "display2/primitives/Button.h"
#include "display2/primitives/Label.h"

#include "display2/utility/DefaultStyle.h"
#include "display2/utility/DefaultSizes.h"
#include "display2/utility/DefaultColors.h"

#include "common/logger.h"

namespace Display {

TopPanel::TopPanel(BaseWidget *parent, Context *ctx) : 
    BaseWidget(parent, true), 
    _ctx(ctx)
{
    setPos(0, 0);
    setSize(parent->width(), DefaultSize::TopPanelHeight);
    lv_obj_set_style_bg_color(_lvhost, DefaultColors::Panels, LV_PART_MAIN);
    lv_obj_add_style(_lvhost, &DefaultStyle::Borderless, 0);

    _btnGrid = std::make_unique<Button>(this, "Grid");
    _btnGrid->setPos(500, 5);
    _btnGrid->tapCallback([](const GestLib::TapGesture &tap) {

    });

    _lblProjectName = std::make_unique<Label>(this, "Untitled");
    _lblProjectName->setPos(10, 35);
}

TopPanel::~TopPanel() {}

}