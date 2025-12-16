// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modules/Mixer/MixerUI.h"

#include "modules/Mixer/MixerView.h"
#include "ui/display/layoutSizes.h"
#include "ui/display/defaultColors.h"
#include "ui/display/primitives/Button.h"
#include "ui/display/primitives/Label.h"
#include "ui/display/primitives/UIContext.h"
#include "ui/display/primitives/Slider.h"
#include "ui/uiutility.h"

#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"
#include "core/primitives/AudioRoute.h"
#include "core/utility/helper.h"
#include "core/Events.h"
#include "logger.h"

#include <vector>

namespace UI {

MixerUI::MixerUI(slr::AudioUnitView * mixer, UIContext * uictx) :
    UnitUIBase(mixer, uictx),
    _mixer(static_cast<slr::MixerView*>(mixer))
{

}

MixerUI::~MixerUI() {

}

bool MixerUI::create(UIContext * ctx) {
    _gridControl = new MixerGridControlUI(ctx->gridControl(), this);
    _moduleUI = new MixerModuleUI(ctx->moduleView(), this);
    return true;
}

bool MixerUI::update(UIContext * ctx) {
    if(_mixer->mute()) {
        _gridControl->_btnMute->setColor(MUTE_ON_COLOR);
    } else {
        _gridControl->_btnMute->setColor(MUTE_OFF_COLOR);
    }

    _gridControl->_lblVolume->setText(std::to_string(_mixer->volume()));

    UnitUIBase::update(ctx);
    return true;
}

bool MixerUI::destroy(UIContext * ctx) {
    UnitUIBase::destroy(ctx);
    delete _gridControl;
    delete _moduleUI;
    return true;
}

// int MixerUI::gridY() {
//     return lv_obj_get_y(_gridControl->lvhost());
// }

// void MixerUI::setNudge(slr::frame_t nudge, const float horizontalZoom) {

// }

// void MixerUI::updatePosition(int x, int y) {
//     lv_obj_set_pos(_gridControl->lvhost(), x, y);
//     // LOG_INFO("TODO: for items: update pos y");
//     // for(FileView * fw : _viewItems) {
//     //     int cx = fw->getX();
//     //     // int cy = lv_obj_get_y(fw->_canvas);
//     //     int cy = fw->getY();
//     //     int newy = y;
//     //     // lv_obj_set_y(fw->_canvas, y);
//     //     fw->setPos(cx, newy);
//     // }
// }

MixerUI::MixerGridControlUI::MixerGridControlUI(BaseWidget *parent, MixerUI *parentUI) :
    BaseWidget(parent, true),
    _parentUI(parentUI)
{
    _flags.isDoubleTap = true;

    setSize(LayoutDef::TRACK_CONTROL_PANEL_WIDTH, LayoutDef::TRACK_HEIGHT);
    int y = LayoutDef::calcTrackY(_parentUI->_uictx->_unitsUI.size());
    setPos(0, y);

    lv_obj_set_style_bg_color(lvhost(),
                    lv_color_make(_parentUI->_mixer->color().r, 
                                    _parentUI->_mixer->color().g, 
                                    _parentUI->_mixer->color().b), 0);

    _lblName = new Label(this, _parentUI->_mixer->name().c_str());
    _lblName->setSize(LayoutDef::TRACK_NAME_LABEL_W, lv_font_get_line_height(&DEFAULT_FONT));
    _lblName->setPos(LayoutDef::TRACK_NAME_LABEL_X, LayoutDef::TRACK_NAME_LABEL_Y);
    _lblName->setFont(&DEFAULT_FONT);
    _lblName->setTextColor(lv_color_hex(0xffffff));
    _lblName->setHoldCallback([this]() {
        this->_parentUI->_uictx->_popManager->enableKeyboard(
            this->_parentUI->_mixer->name(), 
            [this](const std::string & text) {
                LOG_INFO("New mixer name: %s for id: %d", text.c_str(), this->_parentUI->_mixer->id());
                this->_parentUI->_mixer->setName(text);
                this->_lblName->setText(text);
            }
        );
    });

    _lblVolume = new Label(this, std::to_string(_parentUI->_mixer->volume()));
    _lblVolume->setSize(80, lv_font_get_line_height(&DEFAULT_FONT));
    _lblVolume->setPos(280, LayoutDef::TRACK_NAME_LABEL_Y);
    _lblVolume->setFont(&DEFAULT_FONT);
    _lblVolume->setTapCallback([this]() {
        this->_parentUI->_uictx->_popManager->enableKeyboard(
            std::to_string(this->_parentUI->_mixer->volume()),
            [this](const std::string & text) {
                float vol = std::stof(text);
                LOG_WARN("No check for volume!");
                LOG_INFO("Setting volume to: %f", vol);
                slr::Events::SetParameter e = {
                    e.targetId = this->_parentUI->_mixer->id(),
                    e.parameterId = this->_parentUI->_mixer->volumeId(),
                    e.value = vol
                };
                slr::EmitEvent(e);
            }
        );
    });



    int posx = 10;
    int posy = 50;
    _btnMute = new Button(this, "M");
    _btnMute->setPos(posx, posy);
    _btnMute->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnMute->setFont(&lv_font_montserrat_40);
    _btnMute->setCallback([this]() {
        // std::cout << "Mute track: " << (int)_track->id() << " parid: " << _track->muteId() << std::endl;
        slr::Events::SetParameter e = {
            .targetId = _parentUI->_mixer->id(),
            .parameterId = _parentUI->_mixer->muteId(),
            .value = (_parentUI->_mixer->mute() ? slr::boolToFloat(false) : slr::boolToFloat(true))
        };
        slr::EmitEvent(e);
    });

    posx += (LayoutDef::DEFAULT_MARGIN + LayoutDef::BUTTON_SIZE);
    _btnSolo = new Button(this, "S");
    _btnSolo->setPos(posx, posy);
    _btnSolo->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnSolo->setFont(&lv_font_montserrat_40);
    _btnSolo->setCallback([this]() {
        // std::cout << "Solo track: " << (int)_track->id() << " parid: " << "1" << std::endl;
    });

    show();
}

MixerUI::MixerGridControlUI::~MixerGridControlUI() {
    delete _btnMute;
    delete _btnSolo;
    delete _lblVolume;
    delete _lblName;
}

bool MixerUI::MixerGridControlUI::handleDoubleTap(GestLib::DoubleTapGesture &dt) {
    _parentUI->_uictx->_popManager->enableUnitControl(_parentUI, this);
    return true;
}

MixerUI::MixerModuleUI::MixerModuleUI(BaseWidget *parent, MixerUI *parentUI) :
    BaseWidget(parent, true),
    _parentUI(parentUI)
{
    setSize(LayoutDef::WORKSPACE_WIDTH, LayoutDef::WORKSPACE_HEIGHT);
    setPos(0, 0);
    slr::MixerView *m = _parentUI->_mixer;
    slr::Color clr = m->color();
    setColor(lv_color_make(clr.r, clr.g, clr.b));

    hide();
}

MixerUI::MixerModuleUI::~MixerModuleUI() {
    for(auto sl : _sliders) {
        delete sl;
    }
}

void MixerUI::MixerModuleUI::show() {
    //update here
    std::vector<slr::AudioRoute> srcs = slr::ProjectView::getProjectView().targetsForId(_parentUI->_mixer->id());

    if(_sliders.size() != srcs.size()) {
        for(auto sl : _sliders) {
            delete sl;
        }
        _sliders.clear();

        const int firstSliderX = 20;
        const int slidersY = 200;
        int posx = firstSliderX;
        int posy = slidersY;
        for(slr::AudioRoute &src : srcs) {
            Slider * sl = new Slider(this);

            slr::AudioUnitView *auv = slr::ProjectView::getProjectView().getUnitById(src._sourceId);
            const slr::Color &clr = auv->color();

            sl->setPos(posx, posy);
            sl->setColor(lv_color_make(clr.r, clr.g, clr.b));
            sl->onChangeCallback([auv](const float value) {
                slr::Events::SetParameter e = {
                    .targetId = auv->id(),
                    .parameterId = auv->volumeId(),
                    .value = value
                };
                slr::EmitEvent(e);
            });
            sl->setCap(auv->volume());
            
            _sliders.push_back(sl);

            UnitUIBase * uibase = nullptr;
            for(UnitUIBase * base : _parentUI->_uictx->_unitsUI) {
                if(auv->id() == base->id()) {
                    uibase = base;
                    break;
                }
            }

            if(uibase) {
                uibase->registerExternalUpdate(
                    [sl, auv]() {
                        const float value = auv->volume();
                        if(std::fabs(value - sl->getCap()) > 0.2) {
                            sl->setCap(value);
                        }
                });
            }

            posx += (200+20);
        }
    }

    BaseWidget::show();
}

}