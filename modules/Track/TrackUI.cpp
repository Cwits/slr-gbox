// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modules/Track/TrackUI.h"

#include "modules/Track/TrackView.h"
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

TrackUI::TrackUI(slr::AudioUnitView * track, UIContext * uictx) 
    : UnitUIBase(track, uictx), 
    _track(static_cast<slr::TrackView*>(track))
{
    _canLoadFiles = true;
}

TrackUI::~TrackUI() {

}
    
bool TrackUI::create(UIContext * ctx) {
    // MainWindow * mw = MainWindow::inst();

    //grab pointers from ctx
    _gridControl = new TrackGridControlUI(ctx->gridControl(), this);
    _moduleUI = new TrackModuleUI(ctx->moduleView(), this);
    return true;
}

bool TrackUI::update(UIContext * ctx) {
     if(_track->record()) {
        _gridControl->_btnRecord->setColor(RED_COLOR);
        _gridControl->_btnSource->show();
    } else {
        _gridControl->_btnRecord->setColor(BUTTON_DEFAULT_COLOR);
        _gridControl->_btnSource->hide();
    }

    if(_track->recordSource() == slr::RecordSource::Audio) {
        _gridControl->_btnSource->setText("Audio");
    } else {
        _gridControl->_btnSource->setText("Midi");
    }

    if(_track->mute()) {
        _gridControl->_btnMute->setColor(MUTE_ON_COLOR);
    } else {
        _gridControl->_btnMute->setColor(MUTE_OFF_COLOR);
    }

    _gridControl->_lblVolume->setText(std::to_string(_track->volume()));
    
    UnitUIBase::update(ctx);
    return true;
}

bool TrackUI::destroy(UIContext * ctx) {
    UnitUIBase::destroy(ctx);
    delete _gridControl;
    delete _moduleUI;
    return true;
}

TrackUI::TrackGridControlUI::TrackGridControlUI(BaseWidget *parent, TrackUI * parentUI)
    : BaseWidget(parent, true),
    _parentUI(parentUI)
{
    _flags.isDoubleTap = true;

    setSize(LayoutDef::TRACK_CONTROL_PANEL_WIDTH, LayoutDef::TRACK_HEIGHT);
    int y = LayoutDef::calcTrackY(_parentUI->_uictx->_unitsUI.size());
    setPos(0, y);

    lv_obj_set_style_bg_color(lvhost(),
                    lv_color_make(_parentUI->_track->color().r, 
                                    _parentUI->_track->color().g, 
                                    _parentUI->_track->color().b), 0);

    _lblName = new Label(this, _parentUI->_track->name().c_str());
    _lblName->setSize(LayoutDef::TRACK_NAME_LABEL_W, lv_font_get_line_height(&DEFAULT_FONT));
    _lblName->setPos(LayoutDef::TRACK_NAME_LABEL_X, LayoutDef::TRACK_NAME_LABEL_Y);
    _lblName->setFont(&DEFAULT_FONT);
    _lblName->setTextColor(lv_color_hex(0xffffff));
    _lblName->setHoldCallback([this]() {
        this->_parentUI->_uictx->_popManager->enableKeyboard(
            this->_parentUI->_track->name(), 
            [this](const std::string & text) {
                LOG_INFO("New track name: %s for id: %d", text.c_str(), this->_parentUI->_track->id());
                this->_parentUI->_track->setName(text);
                this->_lblName->setText(text);
            }
        );
    });

    _lblVolume = new Label(this, std::to_string(_parentUI->_track->volume()));
    _lblVolume->setSize(80, lv_font_get_line_height(&DEFAULT_FONT));
    _lblVolume->setPos(280, LayoutDef::TRACK_NAME_LABEL_Y);
    _lblVolume->setFont(&DEFAULT_FONT);
    _lblVolume->setTapCallback([this]() {
        this->_parentUI->_uictx->_popManager->enableKeyboard(
            std::to_string(this->_parentUI->_track->volume()),
            [this](const std::string & text) {
                float vol = std::stof(text);
                LOG_WARN("No check for volume!");
                LOG_INFO("Setting volume to: %f", vol);
                slr::Events::SetParameter e = {
                    e.targetId = this->_parentUI->_track->id(),
                    e.parameterId = this->_parentUI->_track->volumeId(),
                    e.value = vol
                };
                slr::EmitEvent(e);
                // this->_lblVolume->setText(text);
            }
        );
    });

    // _parentUI->_track->registerOnVolumeChange([this](const float volume) {
    //     this->_lblVolume->setText(std::to_string(volume));
    // });

    // _parentUI->_track->registerIdOnChange(
    //     _parentUI->_track->volumeId(),
    //     [this](const float volume) {
    //         this->_lblVolume->setText(std::to_string(volume));
    //     });


    int posx = 10;
    int posy = 50;
    _btnMute = new Button(this, "M");
    _btnMute->setPos(posx, posy);
    _btnMute->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnMute->setFont(&lv_font_montserrat_40);
    _btnMute->setCallback([this]() {
        // std::cout << "Mute track: " << (int)_track->id() << " parid: " << _track->muteId() << std::endl;
        slr::Events::SetParameter e = {
            .targetId = _parentUI->_track->id(),
            .parameterId = _parentUI->_track->muteId(),
            .value = (_parentUI->_track->mute() ? slr::boolToFloat(false) : slr::boolToFloat(true))
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
    
    posx += (LayoutDef::DEFAULT_MARGIN + LayoutDef::BUTTON_SIZE);
    _btnRecord = new Button(this, "R");
    _btnRecord->setPos(posx, posy);
    _btnRecord->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnRecord->setFont(&lv_font_montserrat_40);
    _btnRecord->setCallback([this]() {
        slr::Events::RecordArm e = {
            .targetId = _parentUI->_track->id(),
            .recordState = (_parentUI->_track->record() ? 0.0f : 1.0f),
            .recordSource = (_parentUI->_track->recordSource() == slr::RecordSource::Audio) ? 
                                slr::RecordSource::Audio : 
                                slr::RecordSource::Midi
        };
        slr::EmitEvent(e);
    });
    
    posx += (LayoutDef::DEFAULT_MARGIN + LayoutDef::BUTTON_SIZE);
    _btnSource = new Button(this, "Audio");
    _btnSource->setPos(posx, posy);
    _btnSource->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnSource->setFont(&lv_font_montserrat_20);
    _btnSource->setCallback([this]() {
        slr::TimelineView & tl = slr::TimelineView::getTimelineView();
        
        //prohibit source change during recording
        if(!tl.recording()) {
            slr::Events::RecordArm e = {
                .targetId = _parentUI->_track->id(),
                .recordState = (_parentUI->_track->record() ? 1.0f : 0.0f),
                .recordSource = (_parentUI->_track->recordSource() == slr::RecordSource::Audio) ? 
                                    slr::RecordSource::Midi : 
                                    slr::RecordSource::Audio
            };
            slr::EmitEvent(e);
        }
    });
    _btnSource->hide();


    show();
}

TrackUI::TrackGridControlUI::~TrackGridControlUI() {
    delete _btnMute;
    delete _btnSolo;
    delete _btnRecord;
    delete _btnSource;
    delete _lblVolume;
    delete _lblName;
}
   

bool TrackUI::TrackGridControlUI::handleDoubleTap(GestLib::DoubleTapGesture & dt) {
    LOG_INFO("Double Tap on track controls %i", _parentUI->_track->id());
    _parentUI->_uictx->_popManager->enableUnitControl(_parentUI, this);
    return true;
}

TrackUI::TrackModuleUI::TrackModuleUI(BaseWidget *parent, TrackUI * parentUI) 
    : BaseWidget(parent, true),
    _parentUI(parentUI)
{
    setSize(LayoutDef::WORKSPACE_WIDTH, LayoutDef::WORKSPACE_HEIGHT);
    setPos(0, 0);


    _testRect = lv_obj_create(lvhost());
    lv_obj_set_size(_testRect, 200, 200);
    lv_obj_set_pos(_testRect, 100, 100);
    slr::TrackView *tr = _parentUI->_track;
    slr::Color clr = tr->color();
    lv_obj_set_style_bg_color(_testRect, lv_color_make(clr.r, clr.g, clr.b), 0);
    hide();
}

TrackUI::TrackModuleUI::~TrackModuleUI() {
    lv_obj_delete(_testRect);
}

int TrackUI::gridY() {
    return lv_obj_get_y(_gridControl->lvhost());
}

void TrackUI::setNudge(slr::frame_t nudge, const float horizontalZoom) {
     slr::TimelineView & tl = slr::TimelineView::getTimelineView();
    int pixPerBar = UIUtility::pixelPerBar(horizontalZoom);
    float pixMoved = ( (float)nudge / tl.framesPerBar() ) - ( (int)nudge/tl.framesPerBar() );
    int startBar = (nudge/tl.framesPerBar());

    std::size_t size = _viewItems.size();
    for(std::size_t i=0; i<size; ++i) {
        FileView * item = _viewItems.at(i);
        // int cx = lv_obj_get_x(item->_canvas);
        int cx = item->getY();
        int cy = item->getY();
        int newx = -std::round((pixMoved+startBar)*pixPerBar);
        // lv_obj_set_x(item->_canvas, newx);
        item->setPos(newx, cy);
    }
}

void TrackUI::updatePosition(int x, int y) {
    lv_obj_set_pos(_gridControl->lvhost(), x, y);
    // LOG_INFO("TODO: for items: update pos y");
    for(FileView * fw : _viewItems) {
        int cx = fw->getX();
        // int cy = lv_obj_get_y(fw->_canvas);
        int cy = fw->getY();
        int newy = y;
        // lv_obj_set_y(fw->_canvas, y);
        fw->setPos(cx, newy);
    }
}


}