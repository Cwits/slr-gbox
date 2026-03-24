// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/primitives/FileView.h"

#include "core/primitives/File.h"
#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioPeakFile.h"
#include "core/primitives/AudioPeaks.h"
#include "core/ControlEngine.h"

#include "core/Events.h"

#include "snapshots/ProjectView.h"
#include "snapshots/TimelineView.h"
#include "snapshots/FileContainerView.h"
#include "snapshots/AudioUnitView.h"

#include "ui/display/primitives/UnitUIBase.h"
#include "ui/display/primitives/Button.h"
#include "ui/display/primitives/UIContext.h"
#include "ui/display/GridView.h"
#include "ui/display/layoutSizes.h"
#include "ui/uiutility.h"

#include "logger.h"


static int map(int x, int in_min, int in_max, int out_min, int out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

namespace UI {

FileView::FileView(BaseWidget * parent, UnitUIBase * parentUI, const slr::ClipItemView * clipItem, UIContext * const uictx) :
    BaseWidget(parent, true, true),
    _uictx(uictx),
    _parentUI(parentUI),
    _clipItem(clipItem),
    _uniqueId(clipItem->id())
{
// _lvhost = parent->lvhost();
    _canvas = lv_canvas_create(lvhost());

    _flags.isTap = true;
    _flags.isDoubleTap = true;
    _flags.isHold = true;

    _originalX = 0;
    _originalY = 0;

    //calculate width
    slr::TimelineView & tlsnap = slr::TimelineView::getTimelineView();

    slr::frame_t frames = clipItem->length();// testme.frames();
    int framesPerBar = tlsnap.framesPerBar();
    int pixPerBar = UIUtility::pixelPerBar(_uictx->gridHorizontalZoom());
    float pixelPerFrame = (float)pixPerBar/framesPerBar;
    int pixels = frames * pixelPerFrame;
    
    setSize(pixels, UI::LayoutDef::TRACK_HEIGHT);
    setPos(0, parentUI->gridUI()->gridY());
    lv_obj_set_pos(_canvas, 0, 0);
    
    _peakColor = lv_color_make(parentUI->color().r, 
                                    parentUI->color().g, 
                                    parentUI->color().b);
    _fillColor = lv_color_hex(0x000000);


    _canvasWidth = pixels;
    _canvasHeight = LayoutDef::TRACK_HEIGHT;

    //allocate buffer
    _drawBuffer = new uint8_t[LV_DRAW_BUF_SIZE(pixels, LayoutDef::TRACK_HEIGHT, LV_COLOR_FORMAT_NATIVE)];
    
    //set buffer
    lv_canvas_set_buffer(_canvas, _drawBuffer, _canvasWidth, _canvasHeight, LV_COLOR_FORMAT_NATIVE);

    //draw
    draw();
    show();
}

FileView::~FileView() {
    lv_obj_delete(_canvas);
    delete [] _drawBuffer;
}

void FileView::update() {
    //redraw canvas
}

void FileView::draw() {
    lv_canvas_fill_bg(_canvas, lv_palette_main(LV_PALETTE_GREY), LV_OPA_COVER);

    if(_clipItem->item()->_file->isAudio()) {
        const slr::AudioFile * const afile = static_cast<const slr::AudioFile* const>(_clipItem->item()->_file);
        const slr::AudioPeakFile * peakFile = afile->peaks();

        int xsize = _canvasWidth;
        float ratio = (float)_clipItem->length() / (float)xsize; 
        slr::AudioPeaks::LODLevels nearestLvl = slr::AudioPeaks::pickLevel(ratio);

        {//draw
            const slr::AudioPeaks::PeakData0 * buffer = peakFile->data0();

            int channels = buffer->channels();
            float ratio = (float)buffer->bufferSize()/(float)xsize;

            int heightPerChannel = (LayoutDef::TRACK_HEIGHT/channels);
            int midpoint = 0 + (heightPerChannel/channels);
            // int center = midpoint;//+(heightPerChannel/2);

            if(ratio < 1.0f) {
                //less than sample per pixel
            } else {
                for(int ch=0; ch<channels; ++ch) {
                    for(int x=0; x<xsize; ++x) {
                        int offset = ratio*x;

                        uint8_t min = 255;
                        uint8_t max = 0;
                        for(int off=offset; off<offset+ratio; ++off) {
                            min = std::min(min, (*buffer)[ch][off]);
                            max = std::max(max, (*buffer)[ch][off]);
                        }

                        //TODO: this + after map is odd... need to fix it
                        uint8_t dmin = map(min, 0, 255, 0, heightPerChannel) + (ch == 0 ? 0 : heightPerChannel);
                        uint8_t dmax = map(max, 0, 255, 0, heightPerChannel) + (ch == 0 ? 0 : heightPerChannel);
                        // if(dmax > TRACK_HEIGHT) dmax = TRACK_HEIGHT;
                        for(int y=dmin; y<dmax; ++y) {
                            lv_color_t & color = (y == dmin) ? _peakColor : ( (y == dmax) ? _peakColor : _fillColor );
                            lv_canvas_set_px(_canvas, x, y, color, LV_OPA_COVER);
                        }
                    }
                    
                    // midpoint += heightPerChannel;
                }
            }
        }
    } else if(_clipItem->item()->_file->isMidi()) {
        //draw midi file
    }
}

void FileView::pollUIUpdate() {    
    const slr::ClipItemView * const fview = _clipItem;

    uint64_t version = fview->version();
    if(version == _uiVersion) return;
    _uiVersion = version;

    float xposition = UIUtility::frameToPixel(fview->startPosition(), _uictx->gridHorizontalZoom());
    setPos(xposition, _parentUI->gridUI()->gridY());
}

bool FileView::handleTap(GestLib::TapGesture &tap) {
    // LOG_INFO("Tap from item: id: %d, name: %s, track: %d, name: %s", 
    //             _item->_uniqueId, 
    //             _item->_file->name().c_str(), 
    //             _parentTrack->getSnap()->id(),
    //             _parentTrack->getSnap()->name().c_str());
    return true;
}

bool FileView::handleDoubleTap(GestLib::DoubleTapGesture & dtap) {
    _uictx->_popManager->enableFilePopup(this);
    _uictx->_popManager->setFilePopupPosition(dtap.x, dtap.y);
    return true;
}

bool FileView::handleHold(GestLib::HoldGesture &hold) {
    //for dragging file across timeline
    int cx = hold.x-LayoutDef::TRACK_CONTROL_PANEL_WIDTH;
    int cy = hold.y-LayoutDef::TOP_PANEL_HEIGHT;
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
        slr::Events::ModClipItem e = {
            .clipId = _clipItem->id(),
            .startPosition = res,
            .length = _clipItem->length(),
            .fileStartOffset = _clipItem->fileOffset(),
            .muted = _clipItem->muted()
        };
        slr::EmitEvent(e);
        //emit event
    }
    
    return true;
}

FilePopup::FilePopup(BaseWidget * parent, UIContext * const uictx) :
    Popup(parent, uictx) 
{
    setSize(LayoutDef::FILE_POPUP_W, LayoutDef::FILE_POPUP_H);

    _deleteBtn = new Button(this, LV_SYMBOL_TRASH);
    _deleteBtn->setPos(LayoutDef::FILE_POP_DELETE_BTN_X, LayoutDef::FILE_POP_DELETE_BTN_Y);
    _deleteBtn->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _deleteBtn->setFont(&DEFAULT_FONT);
    _deleteBtn->setCallback([this]() {
        LOG_INFO("Remove item event");
        slr::Events::RemoveClip e {
            .clipId = this->_item->_clipItem->id(),
            .unitId = this->_item->parentUI()->id()
        };

        slr::EmitEvent(e);
        this->_uictx->_popManager->disableFilePopup();
    });
}

FilePopup::~FilePopup() {
    delete _deleteBtn;
}

void FilePopup::update() {

}

void FilePopup::setItem(FileView * item) {
    _item = item;
}

void FilePopup::forcedClose() {

}



}

