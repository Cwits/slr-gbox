// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/SimpleOscillator/SimpleOscUI.h"

#include "units/SimpleOscillator/SimpleOscView.h"
#include "snapshots/AudioUnitView.h"

#include "display/elements/Timeline.h"
#include "display/utility/layoutSizes.h"
#include "display/utility/defaultStyles.h"
#include "display/utility/defaultColors.h"
#include "display/utility/UIContext.h"

#include "display/primitives/Button.h"
#include "display/primitives/Label.h"
#include "display/primitives/FileView.h"
#include "common/uiutility.h"

#include "core/utility/helper.h"
// #include "core/Events.h"

#include "common/logger.h"
#include <cassert>

namespace UI {

SimpleOscUI::SimpleOscUI(const std::shared_ptr<const slr::AudioUnitView> &osc, UIContext * uictx) 
    : UnitUIBase(osc, uictx), 
    _osc(std::dynamic_pointer_cast<const slr::SimpleOscView>(osc))
{
    assert(!_osc.expired() && "Unable to cast pointer");
}

SimpleOscUI::~SimpleOscUI() {

}
    
bool SimpleOscUI::create(UIContext * ctx) {
    _gridControl = std::make_unique<DefaultGridUI>(ctx->gridControl(), this);
    _unitUI = std::make_unique<SimpleOscUnitUI>(ctx->unitView(), this);
    return true;
}

bool SimpleOscUI::destroy(UIContext * ctx) {
    UnitUIBase::destroy(ctx);
    _gridControl.reset();
    _unitUI.reset();
    return true;
}

SimpleOscUI::SimpleOscUnitUI::SimpleOscUnitUI(BaseWidget *parent, SimpleOscUI * parentUI) 
    : DefaultUnitUI(parent, parentUI),
    _parentUI(parentUI)
{
    setSize(Layout::WORKSPACE_WIDTH, Layout::WORKSPACE_HEIGHT);
    setPos(0, 0);


    _testRect = lv_obj_create(lvhost());
    lv_obj_set_size(_testRect, 200, 200);
    lv_obj_set_pos(_testRect, 100, 100);
    const std::shared_ptr<const slr::SimpleOscView> tr = _parentUI->_osc.lock();
    slr::Color clr = tr->color();
    lv_obj_set_style_bg_color(_testRect, lv_color_make(clr.r, clr.g, clr.b), 0);
    hide();
}

SimpleOscUI::SimpleOscUnitUI::~SimpleOscUnitUI() {
    lv_obj_delete(_testRect);
}

}