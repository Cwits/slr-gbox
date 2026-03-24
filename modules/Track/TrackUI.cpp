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
}

TrackUI::~TrackUI() {

}
    
bool TrackUI::create(UIContext * ctx) {
    _gridControl = std::make_unique<TrackGridControlUI>(ctx->gridControl(), this);
    _moduleUI = std::make_unique<TrackModuleUI>(ctx->moduleView(), this);
    return true;
}

bool TrackUI::destroy(UIContext * ctx) {
    UnitUIBase::destroy(ctx);
    _gridControl.reset();
    _moduleUI.reset();
    return true;
}

TrackUI::TrackGridControlUI::TrackGridControlUI(BaseWidget *parent, TrackUI * parentUI)
    : DefaultGridUI(parent, parentUI),
    _parentUI(parentUI)
{
    int posx = 10;
    int posy = 50;
    posx += (LayoutDef::DEFAULT_MARGIN + LayoutDef::BUTTON_SIZE);
    posx += (LayoutDef::DEFAULT_MARGIN + LayoutDef::BUTTON_SIZE);

    _btnRecord = std::make_unique<Button>(this, "R");
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
    
    _btnSource = std::make_unique<Button>(this, "Audio");
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
}

void TrackUI::TrackGridControlUI::pollUIUpdate() {
    DefaultGridUI::pollFileUpdate();

    slr::TrackView * view = _parentUI->_track;
    uint64_t v = view->version();
    if(_customVersion == v) return;
    _customVersion = v;

    DefaultGridUI::pollUIUpdate();

    if(view->record()) {
        _btnRecord->setColor(RED_COLOR);
        _btnSource->show();
    } else {
        _btnRecord->setColor(BUTTON_DEFAULT_COLOR);
        _btnSource->hide();
    }

    if(view->recordSource() == slr::RecordSource::Audio) {
        _btnSource->setText("Audio");
    } else {
        _btnSource->setText("Midi");
    }
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

void TrackUI::TrackModuleUI::pollUIUpdate() {
    slr::TrackView * view = _parentUI->_track;
    if(isSameUIVersion(view->version())) return;
    
}

}