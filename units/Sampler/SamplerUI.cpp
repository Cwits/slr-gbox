// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "units/Sampler/SamplerUI.h"
#include "units/Sampler/SamplerView.h"

#include "display/primitives/Label.h"

#include "display/utility/UIContext.h"
#include "display/utility/DragContext.h"
#include "display/utility/layoutSizes.h"
#include "display/utility/DefaultColors.h"
#include "display/utility/FileToCanvas.h"
#include "display/elements/Timeline.h"
#include "display/primitives/FilePreview.h"

#include "snapshots/AudioUnitView.h"
#include "snapshots/FileContainerView.h"

#include "core/primitives/AudioFile.h"
#include "core/actions/Actions.h"
#include "units/Sampler/SamplerActions.h"

#include "common/uiutility.h"
#include "common/logger.h"

#include <cassert>

namespace UI {

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
    
    _testRect = lv_obj_create(lvhost());
    lv_obj_set_size(_testRect, 200, 200);
    lv_obj_set_pos(_testRect, 100, 10);
    const std::shared_ptr<const slr::SamplerView> tr = _parentUI->_sampler.lock();
    slr::Color clr = tr->color();
    lv_obj_set_style_bg_color(_testRect, lv_color_make(clr.r, clr.g, clr.b), 0);

    _horizontalZoom = 1.0f;
    _timeline = std::make_unique<Timeline>(this, lvhost(), &_horizontalZoom, parentUI->_uictx);
    _timeline->setPos(10, 400);
    _timeline->setSize(Layout::WORKSPACE_WIDTH-20, 400);
    _timeline->rebuildTimeline();

    
    _lblAsset = std::make_unique<Label>(this, "No asset");
    _lblAsset->setPos(500, 200);
    _lblAsset->setSize(300, 60);
    _lblAsset->setFont(&DEFAULT_FONT);
    _lblAsset->setTextColor(lv_color_hex(0xffffff));
    _lastAssetState = false;

    dragCallback(std::bind(&SamplerUI::SamplerUnitUI::handleDrag, this, std::placeholders::_1));
}

SamplerUI::SamplerUnitUI::~SamplerUnitUI() {
    lv_obj_delete(_testRect);
}

void SamplerUI::SamplerUnitUI::pollUIUpdate() {
    const std::shared_ptr<const slr::SamplerView> sampler = _parentUI->_sampler.lock();
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
        _preview->_peakColor = lv_color_hex(0x000000);
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

}