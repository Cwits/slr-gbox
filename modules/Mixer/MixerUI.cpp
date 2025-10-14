// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modules/Mixer/MixerUI.h"

#include "modules/Mixer/MixerView.h"
#include "ui/display/layoutSizes.h"
#include "ui/display/primitives/Button.h"
#include "ui/display/primitives/Label.h"
#include "ui/display/primitives/UIContext.h"
#include "ui/uiutility.h"

#include "core/utility/helper.h"
#include "core/Events.h"
#include "logger.h"

namespace UI {

MixerUI::MixerUI(slr::AudioUnitView * mixer, UIContext * uictx) :
    UnitUIBase(mixer),
    _mixer(static_cast<slr::MixerView*>(mixer)),
    _uictx(uictx)
{

}

MixerUI::~MixerUI() {

}

bool MixerUI::create(UIContext * ctx) {
    return true;
}

bool MixerUI::update(UIContext * ctx) {
    return true;
}

bool MixerUI::destroy(UIContext * ctx) {
    return true;
}

int MixerUI::gridY() {
    return 0;
}

void MixerUI::setNudge(slr::frame_t nudge, const float horizontalZoom) {

}

void MixerUI::updatePosition(int x, int y) {

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
                LOG_INFO("New track name: %s for id: %d", text.c_str(), this->_parentUI->_mixer->id());
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
                // this->_lblVolume->setText(text);
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
    return true;
}

MixerUI::MixerModuleUI::MixerModuleUI(BaseWidget *parent, MixerUI *parentUI) :
    BaseWidget(parent, true),
    _parentUI(parentUI)
{

}

MixerUI::MixerModuleUI::~MixerModuleUI() {

}


}