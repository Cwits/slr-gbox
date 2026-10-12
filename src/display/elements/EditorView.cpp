// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display/elements/EditorView.h"

#include "display/elements/Timeline.h"

#include "display/utility/layoutSizes.h"
#include "display/utility/DefaultStyles.h"
#include "display/utility/DefaultColors.h"
#include "display/utility/UIContext.h"
#include "display/utility/DragContext.h"
#include "display/primitives/UnitUIBase.h"
#include "display/primitives/Label.h"
#include "display/primitives/FilePreview.h"

#include "core/primitives/AudioFile.h"
#include "core/primitives/MidiFile.h"
#include "core/actions/Actions.h"
#include "core/FileWorker.h"
#include "core/ControlEngine.h"

#include "snapshots/FileContainerView.h"
#include "snapshots/TimelineView.h"

#include "common/uiutility.h"
#include "common/logger.h"

#include <cmath>

namespace UI {


EditorView::EditorView(BaseWidget * parent, UIContext * uictx) : 
    BaseWidget(parent, true),
    _uictx(uictx) 
{
    setPos(Layout::WORKSPACE_POSITION_X, Layout::WORKSPACE_POSITION_Y);
    setSize(Layout::WORKSPACE_WIDTH, Layout::WORKSPACE_HEIGHT); 
    noScroll();
    addStyle(&Style::workspace);
    
    dragCallback(std::bind(&EditorView::handleDrag, this, std::placeholders::_1));
    swipeCallback(std::bind(&EditorView::handleSwipe, this, std::placeholders::_1));
    zoomCallback(std::bind(&EditorView::handleZoom, this, std::placeholders::_1));

    _toolPanel = lv_obj_create(lvhost());
    lv_obj_set_size(_toolPanel, Layout::WORKSPACE_WIDTH, 100);
    lv_obj_set_pos(_toolPanel, 0, 0);
    lv_obj_add_style(_toolPanel, &Style::workspace, 0);
    lv_obj_set_style_bg_color(_toolPanel, Colors::Red, 0);

    _lblFilePath = std::make_unique<Label>(this, "Empty");
    _lblFilePath->setPos(10, 140);
    _lblFilePath->setSize(1600, 40);

    _verticalZoom = 1.0f;
    _horizontalZoom = 1.0f;

    _timeline = std::make_unique<Timeline>(this, lvhost(), &_horizontalZoom, _uictx);
    _timeline->setPos(0, 100);
    _timeline->setSize(Layout::WORKSPACE_WIDTH, Layout::WORKSPACE_HEIGHT-100);
    _timeline->rebuildTimeline();
    _timeline->show();

    hide();
}

EditorView::~EditorView() {
    lv_obj_delete(_toolPanel);
}

void EditorView::setEditable(slr::ID fileId) {
    //how to find a file?
    //not a good way, better smth like slr::ProjectView::getProject()->findfileById(fileId);
    const slr::FileWorker * worker = slr::ControlEngine::fileWorker();
    const slr::File * file = worker->findFileById(fileId);
    
    if(!file) {
        std::string text = "Failed to find file with id: ";
        text.append(std::to_string(fileId));
        _lblFilePath->setText(text);
    } else {
        std::string text = "File: ";
        text.append(file->path());
        _lblFilePath->setText(text);

        //the vertical and horizontal zoom must be involved to determine _preview->draw(...) arguments
        slr::frame_t fileFrames = 0; //for timeline
        slr::frame_t drawFrames = 0;
        if(file->isAudio()) {
            _content.type = slr::ClipContentType::Audio;
            _content.audio = static_cast<const slr::AudioFile*>(file)->peaks();
            drawFrames = static_cast<const slr::AudioFile*>(file)->frames();
            fileFrames = drawFrames;
            //show audio tools, hide midi
        } else if(file->isMidi()) {
            _content.type = slr::ClipContentType::Midi;
            _content.midi = static_cast<const slr::MidiFile*>(file)->track(0);

            slr::TimelineView &tl = slr::TimelineView::getTimelineView();
            drawFrames = (_content.midi->midiEvents.back().offset / (float)tl.ppqn()) * tl.framesPerQuater();
            //show midi tools, hide audio
        }
        _content.fileId = file->id();
        
        if(_preview) _preview.reset();
        _preview = std::make_unique<FilePreview>(this, &_content, _uictx);
        _preview->_fillColor = lv_color_hex(0x000000);
        _preview->_peakColor = lv_color_hex(0xffffff);
        _preview->setPos(0, 100);
        _preview->setSize(Layout::WORKSPACE_WIDTH, Layout::WORKSPACE_HEIGHT-100);
        if(file->isAudio())
            _preview->draw(0, drawFrames);
        else if(file->isMidi())
            _preview->draw(0, drawFrames, 1, 127);
        _preview->show();

        //recalculate timeline zoom and rebuild
        
        //need to properly recalculate timeline zoom
        float fpp = UIUtility::samplesPerPixel(_horizontalZoom);
        float framesOnTimeline = Layout::WORKSPACE_WIDTH * fpp; //at current zoom
        float ratio = framesOnTimeline / (float)drawFrames;

        float newZoom = _horizontalZoom / ratio;

        _horizontalZoom = newZoom;
        _timeline->rebuildTimeline();
    }
}

void EditorView::pollUIUpdate() {
}

bool EditorView::handleSwipe(const GestLib::SwipeGesture & swipe) {
   
    return true;
}

bool EditorView::handleDrag(const GestLib::DragGesture &drag) {
   
    return true;
}

bool EditorView::handleZoom(const GestLib::ZoomGesture &zoom) {

    return true;
}

}