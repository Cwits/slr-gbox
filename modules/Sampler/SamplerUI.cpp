// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "modules/Sampler/SamplerUI.h"
#include "modules/Sampler/SamplerView.h"

#include "ui/display/primitives/UIContext.h"
#include "ui/display/primitives/DragContext.h"
#include "ui/display/primitives/Label.h"
#include "ui/display/layoutSizes.h"
#include "ui/display/helpers/AudioFileToCanvas.h"

#include "snapshots/AudioUnitView.h"

#include "core/primitives/AudioFile.h"
#include "core/Events.h"


namespace UI {

SamplerUI::SamplerUI(slr::AudioUnitView * sampler, UIContext * uictx) 
    : UnitUIBase(sampler, uictx), 
    _sampler(static_cast<slr::SamplerView*>(sampler))
{

}

SamplerUI::~SamplerUI() {

}

    
bool SamplerUI::create(UIContext * ctx) {
    _gridControl = std::make_unique<SamplerGridControlUI>(ctx->gridControl(), this);
    _moduleUI = std::make_unique<SamplerModuleUI>(ctx->moduleView(), this);
    return true;
}

bool SamplerUI::destroy(UIContext * ctx) {
    UnitUIBase::destroy(ctx);
    _gridControl.reset();
    _moduleUI.reset();
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

    slr::SamplerView * view = _parentUI->_sampler;
    uint64_t v = view->version();
    if(_customVersion == v) return;
    _customVersion = v;

    DefaultGridUI::pollUIUpdate();
}


SamplerUI::SamplerModuleUI::SamplerModuleUI(BaseWidget *parent, SamplerUI * parentUI) 
    : DefaultModuleUI(parent, parentUI),
    _parentUI(parentUI)
{
    setSize(LayoutDef::WORKSPACE_WIDTH, LayoutDef::WORKSPACE_HEIGHT);
    setPos(0, 0);

    _flags.isDrag = true;
    
    _testRect = lv_obj_create(lvhost());
    lv_obj_set_size(_testRect, 200, 200);
    lv_obj_set_pos(_testRect, 100, 10);
    slr::SamplerView *tr = _parentUI->_sampler;
    slr::Color clr = tr->color();
    lv_obj_set_style_bg_color(_testRect, lv_color_make(255-clr.r, 255-clr.g, 255-clr.b), 0);
    setColor(lv_color_make(clr.r, clr.g, clr.b));

    _lblAsset = std::make_unique<Label>(this, "No asset");
    _lblAsset->setPos(500, 200);
    _lblAsset->setSize(300, 60);
    _lblAsset->setFont(&DEFAULT_FONT);
    _lblAsset->setTextColor(lv_color_hex(0xffffff));
    _lastAssetState = false;

    //canvas
    _canvas = lv_canvas_create(lvhost());
    _peakColor = lv_color_make(parentUI->color().r, 
                                    parentUI->color().g, 
                                    parentUI->color().b);
    _fillColor = lv_color_hex(0x000000);
    _canvasWidth = LayoutDef::WORKSPACE_WIDTH-40;
    _canvasHeight = 600;
    lv_obj_set_pos(_canvas, 20, 280);
    _drawBuffer = new uint8_t[LV_DRAW_BUF_SIZE(_canvasWidth, _canvasHeight, LV_COLOR_FORMAT_NATIVE)];    
    //set buffer
    lv_canvas_set_buffer(_canvas, _drawBuffer, _canvasWidth, _canvasHeight, LV_COLOR_FORMAT_NATIVE);
    lv_canvas_fill_bg(_canvas, lv_palette_main(LV_PALETTE_GREY), LV_OPA_COVER);
}

SamplerUI::SamplerModuleUI::~SamplerModuleUI() {
    lv_obj_delete(_testRect);
    lv_obj_delete(_canvas);
    delete [] _drawBuffer;
}

void SamplerUI::SamplerModuleUI::pollUIUpdate() {
    if(_parentUI->_sampler->asset() && !_lastAssetState) {
        _lastAssetState = true;
        std::string text = _parentUI->_sampler->asset()->name();
        _lblAsset->setText(text);

        const slr::AudioFile * const afile = _parentUI->_sampler->asset();
        lv_canvas_fill_bg(_canvas, lv_palette_main(LV_PALETTE_GREY), LV_OPA_COVER);
        UIHelpers::audioFileToCanvas(
            afile,
            0,
            afile->frames(),
            _canvas,
            _canvasHeight,
            _canvasWidth,
            _peakColor,
            _fillColor
        );
    } else if(!_parentUI->_sampler->asset() && _lastAssetState) {
        _lastAssetState = false;
        _lblAsset->setText("No asset");
        
        lv_canvas_fill_bg(_canvas, lv_palette_main(LV_PALETTE_GREY), LV_OPA_COVER);
    }
}


bool SamplerUI::SamplerModuleUI::handleDrag(GestLib::DragGesture & drag) {
    DragContext & ctx = *_parentUI->_uictx->dragContext();
    if(drag.state == GestLib::GestureState::Start) {

    } else if(drag.state == GestLib::GestureState::Move) {
        if(ctx.dragOnGoing) {
            ctx.updateIconPos(drag.x, drag.y);
        }
    } else if(drag.state == GestLib::GestureState::End) {
        slr::Events::LoadAsset e = {
            .path = *ctx.payload.filePath.path,
            .unitId = _parentUI->_sampler->id()
        };
        slr::EmitEvent(e);
            
        ctx.reset();
    }

    return true;
}

}