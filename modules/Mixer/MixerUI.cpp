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
    _gridControl = std::make_unique<DefaultGridUI>(ctx->gridControl(), this);
    _moduleUI = std::make_unique<MixerModuleUI>(ctx->moduleView(), this);
    return true;
}

bool MixerUI::destroy(UIContext * ctx) {
    UnitUIBase::destroy(ctx);
    _gridControl.reset();
    _moduleUI.reset();
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
