// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "snapshots/AudioUnitView.h"
#include "core/primitives/AudioUnit.h"

#include  "core/actions/Actions.h"

#include "common/logger.h"

namespace slr {

AudioUnitView::AudioUnitView(AudioUnit * au) : 
    _au(au),
    _uniqueId(au->id()) 
    // _solo(au->solo()),

{
    addParameter(_volume = new ParameterFloatView(&au->_volume));
    addParameter(_pan = new ParameterFloatView(&au->_pan));
    addParameter(_mute = new ParameterBoolView(&au->_mute));

    
    _name = "Untitled ";
    _name.append(std::to_string(au->id()));
    _uniqueColor.r = rand()%255;
    _uniqueColor.g = rand()%255;
    _uniqueColor.b = rand()%255;
    _uniqueColor.a = 255;

    _solo = false;

    _version.store(0);
}

AudioUnitView::~AudioUnitView() {
}

void AudioUnitView::addParameter(ParameterBaseView * base) {
    _flatParameterList.add(base);
}

void AudioUnitView::setParameter(ID parameterId, float value) { 
    _flatParameterList.operator[](parameterId)->setValue(value); 
    incrementVersion();
}

void AudioUnitView::appendClipItem(ClipItemView * item) {
    // _clipContainer._items.push_back(item);
    _clipContainer.addClipItem(item);
    incrementVersion();
} 

void AudioUnitView::incrementVersion() {
    _version.fetch_add(1, std::memory_order_acq_rel);
}

uint64_t AudioUnitView::version() const { 
    return _version.load(std::memory_order_acquire);
}

nlohmann::ordered_json AudioUnitView::saveUnit() {
    nlohmann::ordered_json ret;
    
    ret["ID"] = id();
    ret["Type"] = unitType();
    ret["Name"] = name();

    //Parameters
    const ParameterArrayView & pars = allParameters();
    const std::size_t count = pars.count();
    for(std::size_t c=0; c<count; ++c) {
        nlohmann::ordered_json parsub;
        parsub["ID"] = c;
        parsub["Value"] = pars[c]->value();
        ret["Parameters"].push_back(parsub);
    }

    slr::Color color = this->color();
    ret["Color"]["r"] = color.r;
    ret["Color"]["g"] = color.g;
    ret["Color"]["b"] = color.b;
    ret["Color"]["a"] = color.a;
        

    ret["Clips"]["Count"] = _clipContainer.clips().size();
    const std::vector<const ClipItemView*> &clips = _clipContainer.clips();
    //clips
    for(const ClipItemView *c : clips) {
        ret["Clips"]["Clips ID"].push_back(c->id());
    }

    return ret;
}

void AudioUnitView::loadUnit(const nlohmann::json &data) {
    //load Parameters
    for(const auto &parsub : data["Parameters"]) {
        auto act = std::make_unique<Actions::SetParameter>();
        act->targetId = id();
        act->parameterId = parsub["ID"].get<ID>();
        act->value = parsub["Value"].get<float>();
        EmitAction(std::move(act));
    }

    Color color; //colors is not loading properly
    color.r = data["Color"]["r"].get<int>();
    color.g = data["Color"]["g"].get<int>();
    color.b = data["Color"]["b"].get<int>();
    color.a = data["Color"]["a"].get<int>();

    auto act1 = std::make_unique<Actions::SetColor>();
    act1->targetId = id();
    act1->color = color;
    EmitAction(std::move(act1));

    auto act2 = std::make_unique<Actions::SetName>();
    act2->targetId = id();
    act2->newName = data["Name"].get<std::string>();
    EmitAction(std::move(act2));

}

    
}