// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modules/SimpleOscillator/SimpleOscUI.h"

#include "modules/SimpleOscillator/SimpleOscView.h"
#include "snapshots/AudioUnitView.h"

#include "ui/display/Timeline.h"
#include "ui/display/layoutSizes.h"
#include "ui/display/defaultStyles.h"
#include "ui/display/defaultColors.h"
#include "ui/display/primitives/UIContext.h"
#include "ui/display/primitives/Button.h"
#include "ui/display/primitives/Label.h"
#include "ui/display/primitives/FileView.h"
#include "ui/uiutility.h"

#include "core/utility/helper.h"
#include "core/Events.h"

#include "logger.h"

namespace UI {

SimpleOscUI::SimpleOscUI(slr::AudioUnitView * osc, UIContext * uictx) 
    : UnitUIBase(osc, uictx), 
    _osc(static_cast<slr::SimpleOscView*>(osc))
{
}

SimpleOscUI::~SimpleOscUI() {

}
    
bool SimpleOscUI::create(UIContext * ctx) {
    _gridControl = std::make_unique<DefaultGridUI>(ctx->gridControl(), this);
    _moduleUI = std::make_unique<SimpleOscModuleUI>(ctx->moduleView(), this);
    return true;
}

bool SimpleOscUI::destroy(UIContext * ctx) {
    UnitUIBase::destroy(ctx);
    _gridControl.reset();
    _moduleUI.reset();
    return true;
}

SimpleOscUI::SimpleOscModuleUI::SimpleOscModuleUI(BaseWidget *parent, SimpleOscUI * parentUI) 
    : BaseWidget(parent, true),
    _parentUI(parentUI)
{
    setSize(LayoutDef::WORKSPACE_WIDTH, LayoutDef::WORKSPACE_HEIGHT);
    setPos(0, 0);


    _testRect = lv_obj_create(lvhost());
    lv_obj_set_size(_testRect, 200, 200);
    lv_obj_set_pos(_testRect, 100, 100);
    slr::SimpleOscView *tr = _parentUI->_osc;
    slr::Color clr = tr->color();
    lv_obj_set_style_bg_color(_testRect, lv_color_make(clr.r, clr.g, clr.b), 0);
    hide();
}

SimpleOscUI::SimpleOscModuleUI::~SimpleOscModuleUI() {
    lv_obj_delete(_testRect);
}

}