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
#include "core/primitives/MidiPeaks.h"
#include "core/actions/Actions.h"
#include "core/filetasks/FileTasks.h"
#include "core/FileWorker.h"
#include "core/ControlEngine.h"

#include "snapshots/ClipContainerView.h"
#include "snapshots/TimelineView.h"

#include "common/uiutility.h"
#include "common/uiControls.h"
#include "common/Math.h"
#include "common/logger.h"

#include <cmath>

namespace UI {


EditorView::EditorView(BaseWidget * parent, UIContext * uictx) : 
    BaseWidget(parent, true),
    _uictx(uictx),
    _midiEditor(this)
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

    _dragStartedHere = false;
    _zoomRedraw = false;
    _content.fileId = 0;
    _lastDrawed = nullptr;

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
    _lastDrawed = file;
    
    if(!file) {
        std::string text = "Failed to find file with id: ";
        text.append(std::to_string(fileId));
        _lblFilePath->setText(text);
        return;
    }

    std::string text = "File: ";
    text.append(file->path());
    _lblFilePath->setText(text);

    //prepare
    slr::frame_t drawFrames = 0;
    if(file->isAudio()) {
        _content.type = slr::ClipContentType::Audio;
        _content.audio = static_cast<const slr::AudioFile*>(file)->peaks();
        drawFrames = static_cast<const slr::AudioFile*>(file)->frames();
        //show audio tools, hide midi
    } else if(file->isMidi()) {
        _content.type = slr::ClipContentType::Midi;
        // _content.midi = static_cast<const slr::MidiFile*>(file)->track(0);
        _content.midi = static_cast<const slr::MidiFile*>(file)->peaks(0);

        slr::TimelineView &tl = slr::TimelineView::getTimelineView();
        drawFrames = (_content.midi->notes().back().ppqnEnd / (float)tl.ppqn()) * tl.framesPerQuater();
        //show midi tools, hide audio
    }
    _content.fileId = file->id();
    
    //reset all values
    _horizontalZoom = 1.0f;
    _verticalZoom = 1.0f;
    _noteStart = 0;
    _noteEnd = 128;
    _drawStart = 0;
    _drawEnd = drawFrames;


    //redraw canvas
    if(_preview) _preview.reset();
    _preview = std::make_unique<FilePreview>(this, &_content, _uictx);
    _preview->_fillColor = lv_color_hex(0x000000);
    _preview->_peakColor = lv_color_hex(0xffffff);
    _preview->setPos(0, 100+30); //30 for timeline numbers
    int canvasHeight = Layout::WORKSPACE_HEIGHT-(100+30); //30 for timeline numbers and 100 for toolbox
    _preview->setSize(Layout::WORKSPACE_WIDTH, canvasHeight);
    if(file->isAudio())
        _preview->draw(_drawStart, _drawEnd);
    else if(file->isMidi())
        _preview->draw(_drawStart, _drawEnd, _noteStart, _noteEnd);
    _preview->show();

    
    if(file->isAudio()) {
        //hide Midi Note Preview things
    } else if(file->isMidi()) {
        //recalculate Note Preview positions and place them
        //bad thing because positions are calcolated twice here and in preview->draw(...) little bit higher...


    }
    

    //recalculate timeline zoom and rebuild
    float fpp = UIUtility::samplesPerPixel(_horizontalZoom);
    float framesOnTimeline = Layout::WORKSPACE_WIDTH * fpp; //at current zoom
    float ratio = framesOnTimeline / (float)_drawEnd;

    float newZoom = _horizontalZoom / ratio;

    _horizontalZoom = newZoom;
    _timeline->rebuildTimeline();

    //show or hide horizontal lines
    if(file->isAudio()) {
        for(auto &l : _midiEditor._lines) l->hide();
    } else if(file->isMidi()) {
        int notesOnDisplay = _noteEnd - _noteStart;
        float heightPerNote = static_cast<float>(canvasHeight) / notesOnDisplay;

        for (int i = 0; i < notesOnDisplay; ++i) {
            Line *l = _midiEditor._lines[i].get();

            int ypos = Math::round(i * heightPerNote)+130;

            l->position[0] = {0, ypos};
            l->position[1] = {Layout::WORKSPACE_WIDTH, ypos};

            lv_line_set_points(l->line, &l->position[0], 2);

            std::string text = std::to_string(_noteEnd - i - 1);
            lv_label_set_text(l->num, text.c_str());

            lv_obj_set_pos(l->num, 3, ypos-3);
            lv_obj_move_to_index(l->num, -1);

            l->show();
        }
    }
}

void EditorView::pollUIUpdate() {
    
    if(_zoomRedraw) {
        _zoomRedraw = false;
        
        if(_content.fileId != 0)
            _preview->draw(_drawStart, _drawEnd, _noteStart, _noteEnd);

        //recalculate timeline zoom and rebuild
        float fpp = UIUtility::samplesPerPixel(_horizontalZoom);
        float framesOnTimeline = Layout::WORKSPACE_WIDTH * fpp; //at current zoom
        float ratio = framesOnTimeline / (float)_drawEnd;

        float newZoom = _horizontalZoom / ratio;

        // _horizontalZoom = newZoom; file dependent thing
        _timeline->nudge(_drawStart);
        _timeline->rebuildTimeline();

        //show or hide horizontal lines
        if(_content.isAudio()) {
            for(auto &l : _midiEditor._lines) l->hide();
        } else if(_content.isMidi()) {
            int notesOnDisplay = _noteEnd - _noteStart;
            float heightPerNote = static_cast<float>(Layout::WORKSPACE_HEIGHT-(100+30)) / notesOnDisplay;

            if(notesOnDisplay > 128) notesOnDisplay = 128; //???
            for (int i = 0; i < notesOnDisplay; ++i) {
                Line *l = _midiEditor._lines[i].get();

                int ypos = Math::round(i * heightPerNote)+130;

                l->position[0] = {0, ypos};
                l->position[1] = {Layout::WORKSPACE_WIDTH, ypos};

                lv_line_set_points(l->line, &l->position[0], 2);

                std::string text = std::to_string(_noteEnd - i - 1);
                lv_label_set_text(l->num, text.c_str());

                lv_obj_set_pos(l->num, 3, ypos-3);
                lv_obj_move_to_index(l->num, -1);

                l->show();
            }
        }
    }
    
    //check if file updated

}

bool EditorView::handleSwipe(const GestLib::SwipeGesture & swipe) {
   
    return true;
}

bool EditorView::handleDrag(const GestLib::DragGesture &drag) {
    DragContext & ctx = *_uictx->dragContext();
    //emm... how to determine if
    if(drag.state == GestLib::GestureState::Start) {
        _dragStartedHere = true;
        //possibly user want to move midi events around
    } else if(drag.state == GestLib::GestureState::Move) {
        if(!_dragStartedHere) {
            if(ctx.dragOnGoing) {
                ctx.updateIconPos(drag.x, drag.y);
            }   
        } else {
            //possibly user want to move midi events around
        }
    } else if(drag.state == GestLib::GestureState::End) {
        if(!_dragStartedHere) {
            if(ctx.dragOnGoing && ctx.origin != nullptr) {
                if(ctx.payload.type == DragPayload::DataType::FilePath) {
                    std::string *path = ctx.payload.filePath.path;

                    // auto act = std::make_unique<>();
                    auto act = std::make_unique<slr::Tasks::OpenFile>();
                    act->path = *path;
                    act->finished = [this](const slr::File *file, bool success) {
                        //WARN: Not UI Thread anymore!
                        if(!success) {
                            LOG_FAIL("Failed last Open File Task");
                            return;
                        }
                        
                        UIControls::DisplayTask([fileId = file->id(), this]() {
                            this->setEditable(fileId);
                        });

                    };
                    slr::ControlEngine::fileWorker()->addTask(std::move(act));
                }
            }
            ctx.reset();
        } else {
            
        }
        _dragStartedHere = false;
    }

    return true;
}

bool EditorView::handleZoom(const GestLib::ZoomGesture &zoom) {
    
    /*
    1. need to find center in frames based on current range and zoom
    2. calculate new zoom values
    3. update gui (can't force to redraw here i guess, better raise some flag and redraw in pollUIUpdate())
    
    as well as need some limits to prevent zoom going to inf
    */
    
    double newHorZoom = _horizontalZoom / zoom.horDif;
    double newVerZoom = _verticalZoom / zoom.verDif;

    double horDiff = _horizontalZoom / newHorZoom;

    LOG_INFO("Zoom difs h%.2f v%.2f, new h%.2f v%.2f", 
        zoom.horDif, zoom.verDif, newHorZoom, newVerZoom);

    int width = Layout::WORKSPACE_WIDTH;
    int height = Layout::WORKSPACE_HEIGHT-(100+30);

    //need to check for file end
    double anchor = Math::clamp((double)zoom.xCenter / width, 0.0, 1.0);

    slr::frame_t framesOnDisplay = _drawEnd - _drawStart;
    slr::frame_t zoomCenterFrame = _drawStart + framesOnDisplay * anchor; 
    
    slr::frame_t newFrames = framesOnDisplay * newHorZoom;

    _drawStart = zoomCenterFrame - (newFrames * anchor);
    _drawEnd = _drawStart + newFrames;

    // _drawStart = Math::abs((long)framesOnDisplay - (long)newFrames);
    // _drawEnd = newFrames;

    _horizontalZoom = newHorZoom;

    _zoomRedraw = true;

    // _horizontalZoom = newHorZoom;
    // _verticalZoom = newVerZoom;

    return true;
}


EditorView::MidiEditor::MidiEditor(BaseWidget *parent) :
    BaseWidget(parent, false)
{    
    _lines.reserve(128);
    for(int i=0; i<128; ++i) {
        std::unique_ptr<Line> l = std::make_unique<Line>();
        l->line = lv_line_create(parent->lvhost());
        lv_obj_add_style(l->line, &Style::EditorLine, 0);
        if(i == 0) lv_obj_set_style_line_color(l->line, lv_color_hex(0xff0000), 0);

        l->num = lv_label_create(parent->lvhost());
        lv_obj_set_style_text_font(l->num, &lv_font_montserrat_8, 0);
        lv_obj_set_style_text_color(l->num, Colors::White, 0);

        l->hide();
        _lines.push_back(std::move(l));
    }
}

}