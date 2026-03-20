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
    // MainWindow * mw = MainWindow::inst();

    //grab pointers from ctx
    _gridControl = new SimpleOscGridControlUI(ctx->gridControl(), this);
    _moduleUI = new SimpleOscModuleUI(ctx->moduleView(), this);
    return true;
}

// bool SimpleOscUI::update(UIContext * ctx) {
    
//     UnitUIBase::update(ctx);
//     return true;
// }

bool SimpleOscUI::destroy(UIContext * ctx) {
    UnitUIBase::destroy(ctx);
    delete _gridControl;
    delete _moduleUI;
    return true;
}

SimpleOscUI::SimpleOscGridControlUI::SimpleOscGridControlUI(BaseWidget *parent, SimpleOscUI * parentUI)
    : BaseWidget(parent, true),
    _parentUI(parentUI)
{
    _flags.isDoubleTap = true;

    setSize(LayoutDef::TRACK_CONTROL_PANEL_WIDTH, LayoutDef::TRACK_HEIGHT);
    int y = LayoutDef::calcTrackY(_parentUI->_uictx->_unitsUI.size());
    setPos(0, y);

    lv_obj_set_style_bg_color(lvhost(),
                    lv_color_make(_parentUI->_osc->color().r, 
                                    _parentUI->_osc->color().g, 
                                    _parentUI->_osc->color().b), 0);

    _lblName = new Label(this, _parentUI->_osc->name().c_str());
    _lblName->setSize(LayoutDef::TRACK_NAME_LABEL_W, lv_font_get_line_height(&DEFAULT_FONT));
    _lblName->setPos(LayoutDef::TRACK_NAME_LABEL_X, LayoutDef::TRACK_NAME_LABEL_Y);
    _lblName->setFont(&DEFAULT_FONT);
    _lblName->setTextColor(lv_color_hex(0xffffff));
    _lblName->setHoldCallback([this]() {
        this->_parentUI->_uictx->_popManager->enableKeyboard(
            this->_parentUI->_osc->name(), 
            [this](const std::string & text) {
                LOG_INFO("New track name: %s for id: %d", text.c_str(), this->_parentUI->_osc->id());
                this->_parentUI->_osc->setName(text);
                this->_lblName->setText(text);
            }
        );
    });

    _lblVolume = new Label(this, std::to_string(_parentUI->_osc->volume()));
    _lblVolume->setSize(80, lv_font_get_line_height(&DEFAULT_FONT));
    _lblVolume->setPos(280, LayoutDef::TRACK_NAME_LABEL_Y);
    _lblVolume->setFont(&DEFAULT_FONT);
    _lblVolume->setTapCallback([this]() {
        this->_parentUI->_uictx->_popManager->enableKeyboard(
            std::to_string(this->_parentUI->_osc->volume()),
            [this](const std::string & text) {
                float vol = std::stof(text);
                LOG_WARN("No check for volume!");
                LOG_INFO("Setting volume to: %f", vol);
                slr::Events::SetParameter e = {
                    e.targetId = this->_parentUI->_osc->id(),
                    e.parameterId = this->_parentUI->_osc->volumeId(),
                    e.value = vol
                };
                slr::EmitEvent(e);
                // this->_lblVolume->setText(text);
            }
        );
    });

    show();
}

SimpleOscUI::SimpleOscGridControlUI::~SimpleOscGridControlUI() {
    delete _lblVolume;
    delete _lblName;
}
   

bool SimpleOscUI::SimpleOscGridControlUI::handleDoubleTap(GestLib::DoubleTapGesture & dt) {
    LOG_INFO("Double Tap on track controls %i", _parentUI->_osc->id());
    _parentUI->_uictx->_popManager->enableUnitControl(_parentUI, this);
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

// int SimpleOscUI::gridY() {
//     return lv_obj_get_y(_gridControl->lvhost());
// }

// void SimpleOscUI::updatePosition(int x, int y) {
//     lv_obj_set_pos(_gridControl->lvhost(), x, y);
//     // LOG_INFO("TODO: for items: update pos y");
   
// }


}