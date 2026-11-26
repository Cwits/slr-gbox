// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "units/Sampler/SamplerUI.h"
#include "units/Sampler/SamplerView.h"
#include "units/Sampler/SamplerCommon.h"
#include "units/Sampler/SamplerActions.h"

#include "display/primitives/Label.h"
#include "display/primitives/Button.h"
#include "display/primitives/DropDown.h"

#include "display/utility/UIContext.h"
#include "display/utility/DragContext.h"
#include "display/utility/layoutSizes.h"
#include "display/utility/DefaultColors.h"
#include "display/utility/DefaultStyles.h"
#include "display/utility/FileToCanvas.h"
#include "display/utility/Macros.h"
#include "display/elements/Timeline.h"
#include "display/primitives/FilePreview.h"

#include "display/utility/DefaultColors.h"

#include "snapshots/AudioUnitView.h"
#include "snapshots/TimelineView.h"

#include "core/primitives/AudioFile.h"
#include "core/actions/Actions.h"

#include "common/uiutility.h"
#include "common/Math.h"
#include "common/logger.h"

#include <cassert>

namespace UI {

bool _REDRAW = false;

void timer_cb(lv_timer_t * timer) {
    _REDRAW = true;
}

SamplerUI::SamplerUI(const std::shared_ptr<const slr::AudioUnitView> &sampler, UIContext * uictx) 
    : UnitUIBase(sampler, uictx), 
    _sampler(std::dynamic_pointer_cast<const slr::SamplerView>(sampler))
{
    assert(!_sampler.expired() && "Unable to cast pointer");
}

SamplerUI::~SamplerUI() {

}

    
bool SamplerUI::create(UIContext * ctx) {
    _gridControl = std::make_unique<SamplerGridControlUI>(ctx->gridControl(), this);
    _unitUI = std::make_unique<SamplerUnitUI>(ctx->unitView(), this);
    return true;
}

bool SamplerUI::destroy(UIContext * ctx) {
    UnitUIBase::destroy(ctx);
    _gridControl.reset();
    _unitUI.reset();
    return true;
}


SamplerUI::SamplerGridControlUI::SamplerGridControlUI(BaseWidget *parent, SamplerUI * parentUI) 
    : DefaultGridUI(parent, parentUI), 
    _parentUI(parentUI)
{

}

SamplerUI::SamplerGridControlUI::~SamplerGridControlUI() {

}

void SamplerUI::SamplerGridControlUI::pollUIUpdate() {
    DefaultGridUI::pollFileUpdate();

    const std::shared_ptr<const slr::SamplerView> view = _parentUI->_sampler.lock();
    uint64_t v = view->version();
    if(_customVersion == v) return;
    _customVersion = v;

    DefaultGridUI::pollUIUpdate();
}


SamplerUI::SamplerUnitUI::SamplerUnitUI(BaseWidget *parent, SamplerUI * parentUI) 
    : DefaultUnitUI(parent, parentUI),
    _parentUI(parentUI)
{
    setSize(Layout::WORKSPACE_WIDTH, Layout::WORKSPACE_HEIGHT);
    setPos(0, 0);
    noScroll();

    // lv_color_t clr = lv_color_hex(parentUI->color().argb32());
    _lblAsset = std::make_unique<Label>(this, "No asset");
    _lblAsset->setPos(10, Layout::Margin);
    _lblAsset->setSize(300, 50);
    _lastAssetState = false;
    
    LABEL(_lblRootPitchStatic, 10, 70, 300, 50, "Root Pitch");
    LABEL(_lblRootPitch, 300, 70, 300, 50, std::to_string(DEFAULT_ROOT_PITCH));
    LABEL(_lblMaxVoicesStatic, 10, 140, 300, 50, "Voice Count");
    LABEL(_lblMaxVoices, 300, 140, 300, 50, std::to_string(DEFAULT_VOICES));
    LABEL(_lblSampleStartStatic, 10, 210, 300, 50, "Sample Start");
    LABEL(_lblSampleStart, 300, 210, 300, 50, "0");
    LABEL(_lblSampleEndStatic, 10, 280, 300, 50, "Sample End");
    LABEL(_lblSampleEnd, 300, 280, 300, 50, "0");

    LABEL(_lblModeStatic, 500, 70, 300, 50, "Mode");
    
    _ddMode = std::make_unique<DropDown>(this);
    _ddMode->setSize(300, 50);
    _ddMode->setPos(600, 70-5);
    _ddMode->setSelected("Classic");
    std::vector<std::string> items; 
    items.push_back("One Shot");
    items.push_back("Slicer");
    _ddMode->setItems(items);
    _ddMode->selectedCallback([this](std::string item) {
        SamplerMode modeToSet;
        if(item.compare("Classic") == 0) {
            //set Classic mode
            modeToSet = SamplerMode::Classic;
        } else if(item.compare("One Shot") == 0) {
            modeToSet = SamplerMode::OneShot;
        } else if(item.compare("Slicer") == 0) {
            modeToSet = SamplerMode::Slicer;
        }
        //emit action...

        auto act = std::make_unique<slr::Actions::SetMode>();
        act->targetId = this->_parentUI->_sampler.lock()->id();
        act->newMode = modeToSet;
        slr::EmitAction(std::move(act));
    });
    
    _horizontalZoom = 1.0f;
    _timeline = std::make_unique<Timeline>(this, lvhost(), &_horizontalZoom, parentUI->_uictx);
    _timeline->setPos(10, 400);
    _timeline->setSize(Layout::WORKSPACE_WIDTH-20, 400);
    _timeline->rebuildTimeline();

    dragCallback(std::bind(&SamplerUI::SamplerUnitUI::handleDrag, this, std::placeholders::_1));

    for(auto &vl : _voiceLines) {
        vl.line = lv_line_create(lvhost()); 
        lv_obj_add_style(vl.line, &Style::gridLine, 0);
        lv_obj_set_style_line_color(vl.line, Colors::White, 0);
        vl.points[0] = {0, 0};
        vl.points[1] = {0, 0};
        lv_obj_add_flag(vl.line, LV_OBJ_FLAG_HIDDEN);
    }

    _updateTimer = lv_timer_create(timer_cb, 100,  nullptr);
    lv_timer_set_repeat_count(_updateTimer, -1);
}

SamplerUI::SamplerUnitUI::~SamplerUnitUI() {
    for(auto &vl : _voiceLines) lv_obj_delete(vl.line);
}

void SamplerUI::SamplerUnitUI::pollUIUpdate() {
    const std::shared_ptr<const slr::SamplerView> sampler = _parentUI->_sampler.lock();

    if(_REDRAW) {
        _REDRAW = false;
     
            //check on voices - make quick copy(not thread safe stuff)
        const std::array<slr::Voice, MAX_VOICES> &varr = sampler->voices();

        slr::TimelineView & tl = slr::TimelineView::getTimelineView();
        int pixPerBar = UIUtility::pixelPerBar(_horizontalZoom);
        float framesPerPixel = (float)pixPerBar / tl.framesPerBar();
        
        for(int i=0; i<MAX_VOICES; ++i) {
            const auto &v = varr[i];
            auto &vl = _voiceLines[i];

            if(!v.active) {
                lv_obj_add_flag(vl.line, LV_OBJ_FLAG_HIDDEN);
                continue;
            }

            float res = Math::round(framesPerPixel*(v.playbackPosition-_timeline->nudge()));
            int posx = res + 10;
            
            vl.active = true;

            int diff = vl.points[0].x - posx;
            if(Math::abs(diff) < 2) continue;

            vl.points[0] = {posx, 430};
            vl.points[1] = {posx, 430+400};

            lv_line_set_points(vl.line, vl.points, 2);
            lv_obj_clear_flag(vl.line, LV_OBJ_FLAG_HIDDEN);
        } 
    }

    if(sampler->asset()  && !_lastAssetState) {
        _lastAssetState = true;
        std::string text = sampler->asset()->name();
        _lblAsset->setText(text);

        const slr::AudioFile * const afile = sampler->asset();
        _content.type = slr::ClipContentType::Audio;
        _content.audio = afile->peaks();
        _content.fileId = afile->id();

        _preview = std::make_unique<FilePreview>(this, &_content, _parentUI->_uictx);
        _preview->_fillColor = lv_color_make(_parentUI->color().r, 
                                    _parentUI->color().g, 
                                    _parentUI->color().b);
        _preview->_peakColor = Colors::White;
        _preview->setSize(Layout::WORKSPACE_WIDTH-20, 400);
        _preview->setPos(10, 430);
        _preview->draw(0, afile->frames());
        _preview->show();

        //need to properly recalculate timeline zoom
        float fpp = UIUtility::samplesPerPixel(_horizontalZoom);
        float framesOnTimeline = (Layout::WORKSPACE_WIDTH-20) * fpp; //at current zoom
        
        slr::frame_t fileFrames = afile->frames();
        float ratio = framesOnTimeline / (float)fileFrames;

        float newZoom = _horizontalZoom / ratio;

        _horizontalZoom = newZoom;
        _timeline->rebuildTimeline();
    } else if(!sampler->asset() && _lastAssetState) {
        _lastAssetState = false;
        _lblAsset->setText("No asset");
        
        // lv_canvas_fill_bg(_canvas, lv_palette_main(LV_PALETTE_GREY), LV_OPA_COVER);
    }


    if(compareAndSave(sampler->version())) {
        _lblRootPitch->setText(std::to_string(*sampler->_rootPitch));
        _lblMaxVoices->setText(std::to_string(*sampler->_maxVoices));
        _lblSampleStart->setText(std::to_string(*sampler->_sampleStart));
        _lblSampleEnd->setText(std::to_string(*sampler->_sampleEnd));
    

        std::vector<std::string> items;
        std::string selected;
        switch(sampler->_mode) {
            case(SamplerMode::Classic): {
                selected = "Classic";
                items.push_back("One Shot");
                items.push_back("Slicer");
            } break;
            case(SamplerMode::OneShot): {
                selected = "One Shot";
                items.push_back("Classic");
                items.push_back("Slicer");
            } break;
            case(SamplerMode::Slicer): {
                selected = "Slicer";
                items.push_back("Classic");
                items.push_back("One Shot");
            } break;
        }
        _ddMode->setItems(items);
        _ddMode->setSelected(selected);
    }

}


bool SamplerUI::SamplerUnitUI::handleDrag(const GestLib::DragGesture & drag) {
    DragContext & ctx = *_parentUI->_uictx->dragContext();
    if(drag.state == GestLib::GestureState::Start) {

    } else if(drag.state == GestLib::GestureState::Move) {
        if(ctx.dragOnGoing) {
            ctx.updateIconPos(drag.x, drag.y);
        }
    } else if(drag.state == GestLib::GestureState::End) {
        auto act = std::make_unique<slr::Actions::LoadAsset>();
        act->data = *ctx.payload.filePath.path;
        act->targetId = _parentUI->_sampler.lock()->id();
        slr::EmitAction(std::move(act));

        ctx.reset();
    }

    return true;
}

} //namespace UI