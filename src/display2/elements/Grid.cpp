// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display2/elements/Grid.h"

#include "display2/primitives/Button.h"
#include "display2/primitives/Label.h"
#include "display2/primitives/Context.h"

#include "display2/utility/DefaultStyle.h"
#include "display2/utility/DefaultSizes.h"
#include "display2/utility/DefaultColors.h"

#include "common/logger.h"

namespace Display {

Grid::Grid(BaseWidget *parent, Context *ctx) : 
    BaseWidget(parent, true),
    _ctx(ctx)
{
    setPos(0, DefaultSize::TopPanelHeight);
    setSize(parent->width(), DefaultSize::WorkspaceHeight);
    lv_obj_add_style(_lvhost, &DefaultStyle::Workspace, 0);
    lv_obj_set_style_bg_color(_lvhost, DefaultColors::Red, LV_PART_MAIN);


    _btnTphide = std::make_unique<Button>(this, "TP");
    _btnTphide->setPos(200, 200);
    _btnTphide->tapCallback([this](const GestLib::TapGesture &tap) {
        LOG_INFO("height: %d", this->height());
        if(this->_ctx->topPanelVisible()) this->_ctx->hideTopPanel();
        else this->_ctx->showTopPanel();
    });

    _btnBphide = std::make_unique<Button>(this, "BP");
    _btnBphide->setPos(300, 200);
    _btnBphide->tapCallback([this](const GestLib::TapGesture &tap) {
        if(this->_ctx->bottomPanelVisible()) this->_ctx->hideBottomPanel();
        else this->_ctx->showBottomPanel();
    });
}

Grid::~Grid() {
}

}