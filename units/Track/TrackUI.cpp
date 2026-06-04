// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/Track/TrackUI.h"

#include "units/Track/TrackView.h"
#include "units/Track/TrackActions.h"
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
#include "core/Actions.h"

#include "logger.h"

#include <cassert>

namespace UI {

TrackUI::TrackUI(const std::shared_ptr<const slr::AudioUnitView> &track, UIContext * uictx) 
    : UnitUIBase(track, uictx), 
    _track(std::dynamic_pointer_cast<const slr::TrackView>(track))
{
    assert(!_track.expired() && "Failed to cast pointer");
}

TrackUI::~TrackUI() {

}
    
bool TrackUI::create(UIContext * ctx) {
    _gridControl = std::make_unique<TrackGridControlUI>(ctx->gridControl(), this);
    _unitUI = std::make_unique<TrackUnitUI>(ctx->unitView(), this);
    return true;
}

bool TrackUI::destroy(UIContext * ctx) {
    UnitUIBase::destroy(ctx);
    _gridControl.reset();
    _unitUI.reset();
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
        auto act = std::make_unique<slr::Actions::RecordArm>();
        const std::shared_ptr<const slr::TrackView> track = _parentUI->_track.lock();
        act->targetId = track->id();
        act->recordState = (track->record() ? 0.0f : 1.0f);
        act->recordSource = (track->recordSource() == slr::RecordSource::Audio) ? 
                                slr::RecordSource::Audio : 
                                slr::RecordSource::Midi;
        slr::EmitAction(std::move(act));
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
            
            auto act = std::make_unique<slr::Actions::RecordArm>();
            const std::shared_ptr<const slr::TrackView> track = _parentUI->_track.lock();
            act->targetId = track->id();
            act->recordState = (track->record() ? 1.0f : 0.0f);
            act->recordSource = (track->recordSource() == slr::RecordSource::Audio) ? 
                                    slr::RecordSource::Midi :
                                    slr::RecordSource::Audio; 
            slr::EmitAction(std::move(act));
        }
    });
    _btnSource->hide();


    show();
}

TrackUI::TrackGridControlUI::~TrackGridControlUI() {
}

void TrackUI::TrackGridControlUI::pollUIUpdate() {
    DefaultGridUI::pollFileUpdate();

    const std::shared_ptr<const slr::TrackView> view = _parentUI->_track.lock();
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

TrackUI::TrackUnitUI::TrackUnitUI(BaseWidget *parent, TrackUI * parentUI) 
    : DefaultUnitUI(parent, parentUI),
    _parentUI(parentUI)
{
    setSize(LayoutDef::WORKSPACE_WIDTH, LayoutDef::WORKSPACE_HEIGHT);
    setPos(0, 0);


    _testRect = lv_obj_create(lvhost());
    lv_obj_set_size(_testRect, 200, 200);
    lv_obj_set_pos(_testRect, 100, 100);
    const std::shared_ptr<const slr::TrackView> track = _parentUI->_track.lock();
    slr::Color clr = track->color();
    lv_obj_set_style_bg_color(_testRect, lv_color_make(clr.r, clr.g, clr.b), 0);
    hide();
}

TrackUI::TrackUnitUI::~TrackUnitUI() {
    lv_obj_delete(_testRect);
}

void TrackUI::TrackUnitUI::pollUIUpdate() {
    // slr::TrackView * view = _parentUI->_track;
    const std::shared_ptr<const slr::TrackView> view = _parentUI->_track.lock();
    if(isSameUIVersion(view->version())) return;
    
}

}