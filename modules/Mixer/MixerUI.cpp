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
#include <algorithm>

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

bool MixerUI::destroy(UIContext * ctx) {
    UnitUIBase::destroy(ctx);
    delete _gridControl;
    delete _moduleUI;
    return true;
}


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

void MixerUI::MixerGridControlUI::pollUIUpdate() {
    slr::MixerView *view = _parentUI->_mixer;

    if(isSameUIVersion(view->version())) return;

    if(view->mute()) {
        _btnMute->setColor(MUTE_ON_COLOR);
    } else {
        _btnMute->setColor(MUTE_OFF_COLOR);
    }

    _lblVolume->setText(std::to_string(view->volume()));

    _parentUI->commonUIUpdate();
    // return true;
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
}

void MixerUI::MixerModuleUI::show() {
    
    BaseWidget::show();
}

void MixerUI::MixerModuleUI::pollUIUpdate() {
    //check if there is diff between 
    checkAddOrDeleteSliders();

    for(SliderWork &s : _sliders) {
        //check if need update
        uint64_t version = s.target->version();
        if(version == s._lastTargetVersion) continue;
        s._lastTargetVersion = version;
        
        const float value = s.target->volume();
        if(std::fabs(value - s.slider->getCap()) > 0.2) {
            s.slider->setCap(value);
        }
    }

    //check mixer itself...
    slr::MixerView *view = _parentUI->_mixer;
    if(isSameUIVersion(view->version())) return;

}

void MixerUI::MixerModuleUI::checkAddOrDeleteSliders() {
    const std::vector<slr::AudioRoute> srcs = slr::ProjectView::getProjectView().sourcesForId(_parentUI->_mixer->id());
    
    bool needUpdate = false;
    std::vector<slr::ID> toCreate;

    for(SliderWork &s : _sliders) s.checked = false;

    for(const slr::AudioRoute &r : srcs) {
        bool presented = false;
        for(SliderWork &s : _sliders) {
            if(s.target->id() == r._sourceId) {
                presented = true;
                s.checked = true;
            }
        }
        if(!presented) {
            needUpdate = true;
            toCreate.push_back(r._sourceId);
        }
    }
    
    if(!needUpdate) {
        for(const SliderWork &s : _sliders) {
            if(!s.checked) needUpdate = true;
        }

        if(!needUpdate) return;
    }

    _sliders.erase(std::remove_if(
        _sliders.begin(), 
        _sliders.end(), 
        [](SliderWork &s) {
            return !s.checked;
        }
    ), _sliders.end());

    const int increment = 200+20;
    int sliderXinit = _sliders.empty() ? 20 : (_sliders.back().slider->getX()+increment);
    const int sliderY = 200;

    for(slr::ID &id : toCreate) {
        SliderWork slw;
        slw.slider = std::make_unique<Slider>(this);

        slr::AudioUnitView *auv = slr::ProjectView::getProjectView().getUnitById(id);
        const slr::Color &clr = auv->color();

        slw.slider->setPos(sliderXinit, sliderY);
        slw.slider->setColor(lv_color_make(clr.r, clr.g, clr.b));
        slw.slider->onChangeCallback([auv](const float value) {
            slr::Events::SetParameter e = {
                .targetId = auv->id(),
                .parameterId = auv->volumeId(),
                .value = value
            };
            slr::EmitEvent(e);
        });
        slw.slider->setCap(auv->volume());

        slw.target = auv;
        slw._lastTargetVersion = auv->version();
        slw.checked = false;

        _sliders.push_back(std::move(slw));
        sliderXinit += increment;
    }
}

MixerUI::MixerModuleUI::SliderWork::SliderWork() {}
MixerUI::MixerModuleUI::SliderWork::~SliderWork() {}

}
