// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display/primitives/ClipPreview.h"

#include "core/primitives/File.h"
#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioPeakFile.h"
#include "core/primitives/AudioPeaks.h"
#include "core/primitives/ClipItem.h"
#include "core/primitives/MidiFile.h"
#include "core/ControlEngine.h"
#include "core/SettingsManager.h"

#include  "core/actions/Actions.h"

#include "snapshots/ProjectView.h"
#include "snapshots/TimelineView.h"
#include "snapshots/ClipContainerView.h"
#include "snapshots/AudioUnitView.h"

#include "display/primitives/UnitUIBase.h"
#include "display/primitives/Button.h"
#include "display/primitives/FilePreview.h"
#include "display/elements/GridView.h"
#include "display/utility/layoutSizes.h"
#include "display/utility/DefaultStyles.h"
#include "display/utility/DefaultColors.h"
#include "display/utility/UIContext.h"
#include "display/utility/FileToCanvas.h"
#include "display/utility/Macros.h"
#include "common/uiutility.h"

#include "common/logger.h"

namespace UI {

ClipPreview::ClipPreview(BaseWidget * parent, UnitUIBase * parentUI, const slr::ClipItemView * clipItem, UIContext * const uictx) :
    BaseWidget(parent, true, true),
    _uictx(uictx),
    _parentUI(parentUI),
    _clipItem(clipItem),
    _uniqueId(clipItem->id())
{
    addStyle(&Style::GridFile);
    noScroll();
    
    _preview = std::make_unique<FilePreview>(this, &clipItem->_content, uictx);
    _preview->setPos(0, 0);
    _preview->_peakColor = lv_color_make(parentUI->color().r, 
                                    parentUI->color().g, 
                                    parentUI->color().b);
    _preview->_fillColor = lv_color_make(parentUI->color().r, 
                                    parentUI->color().g, 
                                    parentUI->color().b);

    tapCallback(std::bind(&ClipPreview::handleTap, this, std::placeholders::_1));
    doubleTapCallback(std::bind(&ClipPreview::handleDoubleTap, this, std::placeholders::_1));
    holdCallback(std::bind(&ClipPreview::handleHold, this, std::placeholders::_1));
}

ClipPreview::~ClipPreview() {
}

void ClipPreview::update() {
    //redraw canvas
}

void ClipPreview::setSize(lv_coord_t w, lv_coord_t h) {
    BaseWidget::setSize(w, h);
    _preview->setSize(w, h);
}

void ClipPreview::recalculateWidthAndRedraw() {
    slr::frame_t frames = _clipItem->length();
    float framesPerPixel = UIUtility::framesPerPixel(_uictx->gridHorizontalZoom());
    int pixels = frames * framesPerPixel;
    
    setSize(pixels, _preview->height());
    _preview->setSize(pixels, _preview->height());
    _preview->draw(_clipItem->fileOffset(), _clipItem->length());
    _preview->show();
}

void ClipPreview::draw() {
    _preview->draw(_clipItem->fileOffset(), _clipItem->length());
    _preview->show();
}

void ClipPreview::pollUIUpdate() {    
    const slr::ClipItemView * const fview = _clipItem;

    uint64_t version = fview->version();
    if(version == _uiVersion) return;
    _uiVersion = version;

    float xposition = UIUtility::frameToPixel(fview->startPosition(), _uictx->gridHorizontalZoom());
    setPos(xposition, _parentUI->gridUI()->getY());
    //as well length
}

bool ClipPreview::handleTap(const GestLib::TapGesture &tap) {
    // LOG_INFO("Tap from item: id: %d, name: %s, track: %d, name: %s", 
    //             _item->_uniqueId, 
    //             _item->_file->name().c_str(), 
    //             _parentTrack->getSnap()->id(),
    //             _parentTrack->getSnap()->name().c_str());
    return true;
}

bool ClipPreview::handleDoubleTap(const GestLib::DoubleTapGesture & dtap) {
    _uictx->_popManager->setClipPreviewPopupPosition(dtap.x, dtap.y);
    _uictx->_popManager->enableClipPreviewPopup(this);
    return true;
}

bool ClipPreview::handleHold(const GestLib::HoldGesture &hold) {
    //for dragging file across timeline
    int cx = hold.x-Layout::TRACK_CONTROL_PANEL_WIDTH;
    int cy = hold.y-Layout::TOP_PANEL_HEIGHT;
    if(hold.state == GestLib::GestureState::Start) {
        //grab item
        _originalX = getX();
        _originalY = getY();
        setPos(cx, cy);
    } else if(hold.state == GestLib::GestureState::Move) {
        setPos(cx, cy); 
        //somehow position of element still goes to _originalX\Y on grid... :S
        //i know... it is happening because of UnitUIBase.cpp:115 updates position at 30fps, 
        //that's why see lag with lv_refr_now and why this stage not working properly
        lv_refr_now(nullptr);
    } else if(hold.state == GestLib::GestureState::End) {
        // setPos(_originalX, _originalY);
        setPos(cx, _originalY);
        
        //TODO: snap to grid
        slr::frame_t res = UIUtility::pixelToFrame(cx, _uictx->gridHorizontalZoom());
        LOG_WARN("File sample pos: %lu", res);

        auto action = std::make_unique<slr::Actions::ModifyClipItem>();
        action->clipId = _clipItem->id();
        action->startPosition = res;
        action->length = _clipItem->length();
        action->fileStartOffset = _clipItem->fileOffset();
        action->muted = _clipItem->muted();
        slr::EmitAction(std::move(action));
    }
    
    return true;
}

ClipPreviewPopup::ClipPreviewPopup(BaseWidget * parent, UIContext * const uictx) :
    Popup(parent, uictx) 
{
    setSize(Layout::FILE_POPUP_W, Layout::FILE_POPUP_H);

    BUTTONDEF(_btnDelete, Layout::FILE_POP_DELETE_BTN_X, Layout::FILE_POP_DELETE_BTN_Y, LV_SYMBOL_TRASH);
    _btnDelete->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        // LOG_INFO("Remove item event");
        auto act = std::make_unique<slr::Actions::RemoveClip>();
        act->targetId = this->_item->parentUI()->id();
        act->clipId = this->_item->_clipItem->id();
        slr::EmitAction(std::move(act));
        this->_uictx->_popManager->disableClipPreviewPopup();
        return true;
    });

    BUTTONDEF(_btnEdit, 20 + Layout::Button, 10, LV_SYMBOL_EDIT);
    _btnEdit->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        this->_uictx->edit(this->_item->_clipItem->fileId());
        this->hide();
        return true;
    });
}

ClipPreviewPopup::~ClipPreviewPopup() {
}

void ClipPreviewPopup::update() {

}

void ClipPreviewPopup::setItem(ClipPreview * item) {
    _item = item;
}

void ClipPreviewPopup::forcedClose() {

}



}

