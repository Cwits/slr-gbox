// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/Mixer/MixerUI.h"

#include "units/Mixer/MixerView.h"
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
#include "core/Actions.h"
// #include "core/Events.h"
#include "logger.h"

#include <vector>
#include <algorithm>
#include <cassert>

namespace UI {

MixerUI::MixerUI(const std::shared_ptr<const slr::AudioUnitView> &mixer, UIContext * uictx) :
    UnitUIBase(mixer, uictx),
    _mixer(std::dynamic_pointer_cast<const slr::MixerView>(mixer))
{
    assert(!_mixer.expired() && "Unable to cast pointer");
}

MixerUI::~MixerUI() {

}

bool MixerUI::create(UIContext * ctx) {
    _gridControl = std::make_unique<DefaultGridUI>(ctx->gridControl(), this);
    _unitUI = std::make_unique<MixerUnitUI>(ctx->unitView(), this);
    return true;
}

bool MixerUI::destroy(UIContext * ctx) {
    UnitUIBase::destroy(ctx);
    _gridControl.reset();
    _unitUI.reset();
    return true;
}

MixerUI::MixerUnitUI::MixerUnitUI(BaseWidget *parent, MixerUI *parentUI) :
    DefaultUnitUI(parent, parentUI),
    _parentUI(parentUI)
{
    setSize(LayoutDef::WORKSPACE_WIDTH, LayoutDef::WORKSPACE_HEIGHT);
    setPos(0, 0);
    const std::shared_ptr<const slr::MixerView> m = _parentUI->_mixer.lock();
    slr::Color clr = m->color();
    setColor(lv_color_make(clr.r, clr.g, clr.b));

    hide();
}

MixerUI::MixerUnitUI::~MixerUnitUI() {
}

void MixerUI::MixerUnitUI::show() {
    
    BaseWidget::show();
}

void MixerUI::MixerUnitUI::pollUIUpdate() {
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
    const std::shared_ptr<const slr::MixerView> view = _parentUI->_mixer.lock();
    if(isSameUIVersion(view->version())) return;

}

void MixerUI::MixerUnitUI::checkAddOrDeleteSliders() {
    const std::vector<slr::AudioRoute> srcs = slr::ProjectView::getProjectView().sourcesForId(_parentUI->_mixer.lock()->id());
    
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
            auto act = std::make_unique<slr::Actions::SetParameter>();
            act->targetId = auv->id();
            act->parameterId = auv->volumeId();
            act->value = value;
            slr::EmitAction(std::move(act));
        });
        slw.slider->setCap(auv->volume());

        slw.target = auv;
        slw._lastTargetVersion = auv->version();
        slw.checked = false;

        _sliders.push_back(std::move(slw));
        sliderXinit += increment;
    }
}

MixerUI::MixerUnitUI::SliderWork::SliderWork() {}
MixerUI::MixerUnitUI::SliderWork::~SliderWork() {}

}
