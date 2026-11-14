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
#include "display/primitives/Button.h"
#include "display/utility/Helpers.h"
#include "display/popups/PopupManager.h"
#include "display/popups/TimelinePopup.h"

#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioPeaks.h"
#include "core/primitives/MidiFile.h"
#include "core/primitives/MidiPeaks.h"
#include "core/actions/Actions.h"
#include "core/filetasks/FileTasks.h"
#include "core/FileWorker.h"
#include "core/ControlEngine.h"

#include "snapshots/ClipContainerView.h"
#include "snapshots/TimelineView.h"
#include "snapshots/ProjectView.h"

#include "common/uiutility.h"
#include "common/uiControls.h"
#include "common/Math.h"
#include "common/logger.h"

#include <cmath>

namespace UI {

lv_style_t handleLine;

lv_style_t styleDark;
lv_style_t styleBright;
lv_style_t noteStyle;

const int ToolboxHeight = Layout::Button;
const int DrawableHeight = Layout::WORKSPACE_HEIGHT - (ToolboxHeight+30); //30 comming from Timeline noubers background
const int DrawableY = ToolboxHeight+30;


bool blackKeys[] = { 
    false, true, 
    false, true, 
    false, 
    false, true, 
    false, true, 
    false, true,
    false 
};

slr::frame_t snapToGrid(slr::frame_t frame, const UI::Timeline &tl) {

    return 0;
}

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

    _lblFilePath = std::make_unique<Label>(this, "Empty");
    _lblFilePath->setPos(10, 140);
    _lblFilePath->setSize(1600, 40);
    lv_obj_set_style_bg_opa(_lblFilePath->lvhost(), LV_OPA_TRANSP, 0);

    _horizontalZoom = 1.0f;
    _zoomRedraw = false;
    
    _audioToolbox = std::make_unique<AudioToolbox>(this);
    _audioEditor = std::make_unique<AudioEditor>(this, *this, *_audioToolbox.get());
    _audioEditor->_content.fileId = 0;
    _audioToolbox->_editor = _audioEditor.get();

    _midiToolbox = std::make_unique<MidiToolbox>(this);
    _midiEditor = std::make_unique<MidiEditor>(this, *this, *_midiToolbox.get());

    _timeline = std::make_unique<Timeline>(this, lvhost(), &_horizontalZoom, _uictx);
    _timeline->setPos(0, ToolboxHeight); //30 from self number background
    _timeline->setSize(Layout::WORKSPACE_WIDTH, Layout::WORKSPACE_HEIGHT-(ToolboxHeight));
    _timeline->rebuildTimeline();
    _timeline->show();

    //todo: dirty hack for tests, remove later
    _enteringViewClb = [this]() {
        this->_uictx->_popManager->_timelinePopup->_current = this->_timeline.get();
    };

    hide();
}

EditorView::~EditorView() {
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
        return;
    }

    std::string text = "File: ";
    text.append(file->path());
    _lblFilePath->setText(text);

    //prepare
    slr::frame_t drawFrames = 0;
    if(file->isAudio()) {
        _audioEditor->preparePreview(static_cast<const slr::AudioFile*>(file));
        
        //show audio tools, hide midi
        _audioEditor->show();
        _audioToolbox->show();
        _midiToolbox->hide();
        _midiEditor->hide();
    } else if(file->isMidi()) {
        _midiEditor->preparePreview(static_cast<const slr::MidiFile*>(file));
        
        //show midi tools, hide audio
        _audioEditor->hide();
        _audioToolbox->hide();
        _midiToolbox->show();
        _midiEditor->show();
    }

    //since we setted up file -> the _drawStart and _drawEnd are filled
    calcNewZoom(_drawEnd);
    _timeline->nudge(_drawStart);
    _timeline->rebuildTimeline();
    // recalculateTimelineHZoom();
}

void EditorView::pollUIUpdate() {
    
    if(_zoomRedraw) {
        _zoomRedraw = false;
        
        //rebuild timeline first, because things below depends on it
        _timeline->nudge(_drawStart);
        _timeline->rebuildTimeline();
        
        if(_audioEditor->visible()) {
            _audioEditor->redraw();
        } else {
            _midiEditor->redraw();
        }
    }
    
    //check if file updated
    if(_audioEditor->visible())
        _audioEditor->_preview->pollUIUpdate();
    

}

void EditorView::calcNewZoom(slr::frame_t size) {
    float fpp = UIUtility::samplesPerPixel(_horizontalZoom);
    float framesOnTimeline = Layout::WORKSPACE_WIDTH * fpp; //at current zoom
    float ratio = framesOnTimeline / (float)size;
    float newZoom = _horizontalZoom / ratio;
    _horizontalZoom = newZoom;
}

bool EditorView::handleSwipe(const GestLib::SwipeGesture & swipe) {
    //when moving from left to right - delta < 0 -- need to go closer to start
    //when from right to left - delta > 0 - need to go closed to end

    if(swipe.state == GestLib::GestureState::End) _zoomRedraw = true;

    int mul = ((float)160 * (1.0f - _horizontalZoom)) * swipe.dx;
    slr::frame_t oldNudge = _drawStart;
    if(swipe.dx > 0) {
       slr::frame_t onDisplay = _drawEnd - _drawStart;
        // LOG_INFO("start: %lu, end: %lu, total: %lu", _drawStart, _drawEnd, _fileLength);
        UIUtility::incrementUntil<slr::frame_t>(_drawEnd, mul, _fileLength, 
            [&start =  _drawStart, &end = _drawEnd, &redraw = _zoomRedraw](const slr::frame_t inc) {
                if(inc == 0) return;
                start += inc;
                end += inc;
                redraw = true;
            }
        );
    } else if(swipe.dx < 0) {
        UIUtility::decrementUntil<slr::frame_t>(_drawStart, std::abs(mul), 0,
            [&start = _drawStart, &end = _drawEnd, &redraw = _zoomRedraw](const slr::frame_t dec) {
                if(dec == 0) return;
                start -= dec;
                end -= dec;
                redraw = true;
            }
        );
    }

    if(!_midiEditor->visible()) return true;

    // LOG_INFO("delta for y %d", swipe.dy);
    static int slowMeDown = 0;
    slowMeDown += std::abs(swipe.dy);
    if(slowMeDown < 30) return true;
    slowMeDown = 0;

    mul = 1;
    
    int &noteMin = _midiEditor->_noteMin;
    int &noteMax = _midiEditor->_noteMax;
    if(swipe.dy > 0) { 
        UIUtility::decrementUntil<int>(noteMin, mul, 0, 
            [&min = noteMin, &max = noteMax, 
            &fmin = _midiEditor->_fNoteMin, &fmax = _midiEditor->_fNoteMax,
            &redraw = _zoomRedraw](const int dec) {
                if(dec == 0) return;
                min -= dec;
                max -= dec;
                fmin -= dec;
                fmin -= max;
                redraw = true;
            }
        );
    } else if(swipe.dy < 0) { 
        //from top to bottom -> scroll up
        UIUtility::incrementUntil<int>(noteMax, std::abs(mul), 127, 
            [&min = noteMin, &max = noteMax, 
            &fmin = _midiEditor->_fNoteMin, &fmax = _midiEditor->_fNoteMax,
            &redraw = _zoomRedraw](const int dec) {
                if(dec == 0) return;
                min += dec;
                max += dec;
                fmin += dec;
                fmin += max;
                redraw = true;
            }
        );
    }


    return true;
}

bool EditorView::handleDrag(const GestLib::DragGesture &drag) {
    DragContext & ctx = *_uictx->dragContext();
    //emm... how to determine if
    if(drag.state == GestLib::GestureState::Start) {

    } else if(drag.state == GestLib::GestureState::Move) {
        if(ctx.dragOnGoing) {
            ctx.updateIconPos(drag.x, drag.y);
        }   
    } else if(drag.state == GestLib::GestureState::End) {
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
    }

    return true;
}

bool EditorView::handleZoom(const GestLib::ZoomGesture &zoom) {
    if(zoom.state == GestLib::GestureState::End) _zoomRedraw = true;

    slr::TimelineView &tl = slr::TimelineView::getTimelineView();

    const double width = Layout::WORKSPACE_WIDTH;
    const double height = DrawableHeight;

    const slr::frame_t oldStart = _drawStart;
    const slr::frame_t oldEnd = _drawEnd;

    {
        double newHorZoom = _horizontalZoom / zoom.horDif;
        const double anchor = Math::clamp(static_cast<double>(zoom.xCenter) / width, 0.0, 1.0);
        const long double oldRange = static_cast<long double>(oldEnd) - oldStart;
        const long double anchorFrame = static_cast<long double>(oldStart) + oldRange * anchor;
        const long double newRange = oldRange / zoom.horDif;

        if(newRange >= _fileLength) return false;

        long double newStart = anchorFrame - newRange * anchor;
        long double newEnd = newStart + newRange;

        if (newRange >= _fileLength) {
            newStart = 0;
            newEnd = _fileLength;
        } else {
            if (newStart < 0) {
                newEnd -= newStart;
                newStart = 0;
            }

            if (newEnd > _fileLength) {
                newStart -= newEnd - _fileLength;
                newEnd = _fileLength;
            }
        }

        _drawStart = static_cast<slr::frame_t>(newStart);
        _drawEnd = static_cast<slr::frame_t>(newEnd);

        _horizontalZoom = newHorZoom;
    }

    //for midi
    if(_midiEditor->visible()) {
        if(zoom.verDif == 0.0f) return true;
        double newVerZoom = _midiEditor->_verticalZoom / zoom.verDif;

        const double anchor = Math::clamp(static_cast<double>(zoom.yCenter) / height, 0.0, 1.0);
        const long double oldRange = static_cast<long double>(_midiEditor->_fNoteMax) - _midiEditor->_fNoteMin;
        const long double anchorFrame = _midiEditor->_fNoteMin + oldRange * anchor;
        const long double newRange = Math::clamp(oldRange / zoom.verDif, 4.0L, 127.0L);
        long double newMin = anchorFrame - newRange * anchor;
        newMin = Math::clamp(newMin, 0.0L, 127.0L - newRange);

        _midiEditor->_fNoteMin = newMin;
        _midiEditor->_fNoteMax = newMin + newRange;

        _midiEditor->_noteMin = Math::round(_midiEditor->_fNoteMin);
        _midiEditor->_noteMax = Math::round(_midiEditor->_fNoteMax);
        if(_midiEditor->_noteMax - _midiEditor->_noteMin < 4) {
            _midiEditor->_noteMax = _midiEditor->_noteMin + 4;

            if(_midiEditor->_noteMax > 127) {
                _midiEditor->_noteMax = 127;
                _midiEditor->_noteMin = 123;
            }
        }

        _midiEditor->_verticalZoom = newVerZoom;
    }

    
    _zoomRedraw = true;

    return true;
}

AudioEditor::AudioEditor(BaseWidget *parent, EditorView &editor, AudioToolbox &toolbox) :
    BaseWidget(parent, true),
    _editor(editor),
    _toolbox(toolbox)
{
    setPos(Layout::WORKSPACE_POSITION_X, DrawableY);
    setSize(Layout::WORKSPACE_WIDTH, DrawableHeight); 
    noScroll();
    addStyle(&Style::workspace);

    
    lv_style_init(&handleLine);
    lv_style_set_line_width(&handleLine, 2);
    lv_style_set_line_color(&handleLine, Colors::White);
    lv_style_set_line_rounded(&handleLine, true);
    lv_style_set_margin_all(&handleLine, 0);
    lv_style_set_pad_all(&handleLine, 0);
    lv_style_set_radius(&handleLine, 0);
    lv_style_set_border_width(&handleLine, 0);

    _filler = lv_obj_create(lvhost());
    _lineStart = lv_line_create(lvhost());
    _lineEnd = lv_line_create(lvhost());

    lv_obj_add_style(_filler, &Style::workspace, 0);
    lv_obj_set_style_bg_color(_filler, Colors::White, 0);
    lv_obj_set_style_bg_opa(_filler, LV_OPA_50, 0);
    lv_obj_add_style(_lineStart, &handleLine, 0);
    lv_obj_add_style(_lineEnd, &handleLine, 0);

    lv_obj_move_to_index(_filler, -1);
    lv_obj_move_to_index(_lineStart, -1);
    lv_obj_move_to_index(_lineEnd, -1);
    
    _handleStart = std::make_unique<Handle>(this);
    _handleEnd = std::make_unique<Handle>(this);
    _handleEnd->_endHandle = true;
    
    _handleStart->setZ(10);
    _handleStart->_editor = this;
    _handleEnd->setZ(10);
    _handleEnd->_editor = this;

    hideHandles();

    dragCallback(std::bind(&AudioEditor::handleDrag, this, std::placeholders::_1));
}

AudioEditor::~AudioEditor() {
    lv_obj_delete(_filler);
    lv_obj_delete(_lineStart);
    lv_obj_delete(_lineEnd);
}

void AudioEditor::showHandles() {
    lv_obj_clear_flag(_filler, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(_lineStart, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(_lineEnd, LV_OBJ_FLAG_HIDDEN);
    _handleStart->show();
    _handleEnd->show();
}

void AudioEditor::hideHandles() {
    lv_obj_add_flag(_filler, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(_lineStart, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(_lineEnd, LV_OBJ_FLAG_HIDDEN);
    _handleStart->hide();
    _handleEnd->hide();
}

bool AudioEditor::handleDrag(const GestLib::DragGesture &drag) {
    //make both start and end, on move change filler width
    if(drag.state == GestLib::GestureState::Start) {
        //put start
        _pointsStart[0] = {drag.x, 0};
        _pointsStart[1] = {drag.x, DrawableHeight};
        lv_line_set_points(_lineStart, _pointsStart, 2);
        _pointsEnd[0] = {drag.x, 0};
        _pointsEnd[1] = {drag.x, DrawableHeight};
        lv_line_set_points(_lineEnd, _pointsEnd, 2); //just to hide end line

        lv_obj_set_pos(_filler, drag.x, 0);
        lv_obj_set_size(_filler, drag.x - _pointsStart[0].x, DrawableHeight);

        _handleStart->setPos(drag.x, 0);
        _handleEnd->setPos(drag.x, 0);

        showHandles();

        _selectorStart = _editor._timeline->positionToFrames(drag.x);
        _toolbox._lblStart->setText(std::to_string(_selectorStart));
    } else if(drag.state == GestLib::GestureState::Move) {
        lv_obj_set_size(_filler, drag.x - _pointsStart[0].x, DrawableHeight);
    } else if(drag.state == GestLib::GestureState::End) {
        _pointsEnd[0] = {drag.x, 0};
        _pointsEnd[1] = {drag.x, DrawableHeight};
        lv_line_set_points(_lineEnd, _pointsEnd, 2);

        lv_obj_set_size(_filler, drag.x - _pointsStart[0].x, DrawableHeight);

        _handleEnd->setPos(drag.x - Layout::Button, DrawableHeight - Layout::Button);
        
        _selectorEnd = _editor._timeline->positionToFrames(drag.x);
        _toolbox._lblEnd->setText(std::to_string(_selectorEnd));
    }

    return true;
}


void AudioEditor::redraw() {
    _preview->draw(_editor._drawStart, _editor._drawEnd);

    //redraw handles 
    int startx = _editor._timeline->absoluteFramesToPosition(_selectorStart);
    int endx = _editor._timeline->absoluteFramesToPosition(_selectorEnd);
    bool startHidden = false;
    bool endHidden = false;
    //start handle
    if(_selectorStart < _editor._drawStart) {
        startHidden = true;
        _handleStart->hide();

        lv_obj_add_flag(_lineStart, LV_OBJ_FLAG_HIDDEN);

        lv_obj_set_pos(_filler, 0, 0);
        lv_obj_set_size(_filler, _pointsEnd[0].x, DrawableHeight);
    } else {
        _handleStart->show();
        _handleStart->setPos(startx, 0);

        _pointsStart[0] = {startx, 0};
        _pointsStart[1] = {startx, DrawableHeight};
        lv_line_set_points(_lineStart, _pointsStart, 2);
        lv_obj_clear_flag(_lineStart, LV_OBJ_FLAG_HIDDEN);

        lv_obj_set_pos(_filler, startx, 0);
        lv_obj_set_size(_filler, (endx > Layout::WORKSPACE_WIDTH ? Layout::WORKSPACE_WIDTH : endx) - startx, DrawableHeight);
    }

    if(_selectorEnd > _editor._drawEnd) {
        //hide
        endHidden = true;
        _handleEnd->hide();

        lv_obj_add_flag(_lineEnd, LV_OBJ_FLAG_HIDDEN);

        lv_obj_set_size(_filler, Layout::WORKSPACE_WIDTH - _pointsStart[0].x, DrawableHeight);
    } else {
        //show
        int posx = _editor._timeline->absoluteFramesToPosition(_selectorEnd);
        _handleEnd->show();
        _handleEnd->setPos(endx - Layout::Button, DrawableHeight - Layout::Button);
        
        _pointsEnd[0] = {endx, 0};
        _pointsEnd[1] = {endx, DrawableHeight};
        lv_line_set_points(_lineEnd, _pointsEnd, 2);
        lv_obj_clear_flag(_lineEnd, LV_OBJ_FLAG_HIDDEN);

        lv_obj_set_size(
            _filler, 
            (startx > 0 ? 
                endx - startx : 
                (endx > Layout::WORKSPACE_WIDTH ? 
                    Layout::WORKSPACE_WIDTH : 
                    endx)
                ), 
            DrawableHeight
        );
    }

    if(startHidden && endHidden) {
        lv_obj_set_pos(_filler, 0, 0);
        lv_obj_set_size(_filler, Layout::WORKSPACE_WIDTH, DrawableHeight);
    }
}

void AudioEditor::preparePreview(const slr::AudioFile *file) {
    _content.type = slr::ClipContentType::Audio;
    _content.audio = file->peaks();
    _content.fileId = file->id();
    //set editor vars
    _editor._drawStart = 0;
    _editor._drawEnd = file->frames();
    _editor._fileLength = _editor._drawEnd;
    
    //redraw canvas
    if(_preview) _preview.reset();
    _preview = std::make_unique<FilePreview>(this, &_content, _editor._uictx);
    _preview->_fillColor = lv_color_hex(0x000000);
    _preview->_peakColor = lv_color_hex(0xffffff);
    _preview->setPos(0, 0); 
    _preview->setSize(Layout::WORKSPACE_WIDTH, DrawableHeight);
    _preview->draw(_editor._drawStart, _editor._drawEnd);
    _preview->show();
    _preview->setZ(1);
}

AudioEditor::Handle::Handle(BaseWidget *parent) :
    BaseWidget(parent, true)
{
    setSize(Layout::Button, Layout::Button);
    touchDownCallback([](const GestLib::TouchDownEvent &td) -> bool {
        return true;
    });
    dragCallback(std::bind(&AudioEditor::Handle::handleDrag, this, std::placeholders::_1));
}

bool AudioEditor::Handle::handleDrag(const GestLib::DragGesture &drag) {
    if(!_endHandle) {
        //start one
        setPos(drag.x, 0);
        _editor->_pointsStart[0] = {drag.x, 0};
        _editor->_pointsStart[1] = {drag.x, DrawableHeight};
        lv_line_set_points(_editor->_lineStart, _editor->_pointsStart, 2);

        lv_obj_set_pos(_editor->_filler, drag.x, 0);
        lv_obj_set_size(_editor->_filler, _editor->_pointsEnd[0].x - drag.x, DrawableHeight);
    } else {
        //end one
        setPos(drag.x - Layout::Button, DrawableHeight - Layout::Button);

        _editor->_pointsEnd[0] = {drag.x, 0};
        _editor->_pointsEnd[1] = {drag.x, DrawableHeight};
        lv_line_set_points(_editor->_lineEnd, _editor->_pointsEnd, 2);

        // lv_obj_set_pos(_editor->_filler, )
        lv_obj_set_size(_editor->_filler, drag.x - _editor->_pointsStart[0].x, DrawableHeight);
    }

    
    if(drag.state == GestLib::GestureState::End) {
        if(_endHandle) {
            _editor->_selectorEnd = _editor->_editor._timeline->positionToFrames(drag.x);
            _editor->_toolbox._lblEnd->setText(std::to_string(_editor->_selectorEnd));
        } else {
            _editor->_selectorStart = _editor->_editor._timeline->positionToFrames(drag.x);
            _editor->_toolbox._lblStart->setText(std::to_string(_editor->_selectorStart));
        }
    }
    return true;
}

AudioToolbox::AudioToolbox(BaseWidget *parent) :
    BaseWidget(parent, true) 
{
    setPos(0, 0);
    setSize(Layout::WORKSPACE_WIDTH, ToolboxHeight);
    noScroll();
    addStyle(&Style::workspace);

    _btnSelectAll = std::make_unique<Button>(this, "SelAll");
    _btnSelectAll->setPos(10, 0);
    _btnSelectAll->tapCallback([&edit = _editor](const GestLib::TapGesture &tap) -> bool {
        edit->_selectorStart = 0;
        edit->_selectorEnd = edit->_editor._fileLength;
        return true;
    });

    _btnCut = std::make_unique<Button>(this, "Cut");
    _btnCut->setPos(Layout::Button + 30, 0);
    _btnCut->tapCallback([&edit = _editor](const GestLib::TapGesture &tap) -> bool {
        
        return true;
    });

    _btnCopy = std::make_unique<Button>(this, "Copy");
    _btnCopy->setPos((Layout::Button*2) + (30*2), 0);
    _btnCopy->tapCallback([&edit = _editor](const GestLib::TapGesture &tap) -> bool {
        auto act = std::make_unique<slr::Actions::CopyAudio>();
        act->start = edit->_selectorStart;
        act->end = edit->_selectorEnd;
        act->fileId = edit->_content.fileId;
        slr::EmitAction(std::move(act));
        return true;
    });

    _btnPaste = std::make_unique<Button>(this, "Paste");
    _btnPaste->setPos((Layout::Button*3) + (30*3), 0);
    _btnPaste->tapCallback([&edit = _editor](const GestLib::TapGesture &tap) -> bool {
        auto act = std::make_unique<slr::Actions::PasteAudio>();
        act->start = edit->_selectorStart;
        act->end = edit->_selectorEnd;
        act->fileId = edit->_content.fileId;
        act->finished = [edit]() {
            UIControls::DisplayTask([edit]() {
                //need to update content...
                const slr::File *f = slr::ControlEngine::fileWorker()->findFileById(edit->_content.fileId);
                edit->_editor._fileLength = f->frames();
                if(edit->_editor._drawEnd > f->frames()) edit->_editor._drawEnd = f->frames();
                if(edit->_selectorStart > f->frames()) edit->_selectorStart = 0;
                if(edit->_selectorEnd > f->frames()) edit->_selectorEnd = 0;

                edit->redraw();
            });
        };
        slr::EmitAction(std::move(act));
        return true;
    });

    _btnDelete = std::make_unique<Button>(this, "Delete");
    _btnDelete->setPos((Layout::Button*4) + (30*4), 0);
    _btnDelete->tapCallback([&edit = _editor](const GestLib::TapGesture &tap) -> bool {
        auto act = std::make_unique<slr::Actions::DeleteAudio>();
        act->start = edit->_selectorStart;
        act->end = edit->_selectorEnd;
        act->fileId = edit->_content.fileId;
        act->finished = [edit]() {
            UIControls::DisplayTask([edit]() {
                const slr::File *f = slr::ControlEngine::fileWorker()->findFileById(edit->_content.fileId);
                edit->_editor._fileLength = f->frames();
                if(edit->_editor._drawEnd > f->frames()) edit->_editor._drawEnd = f->frames();
                if(edit->_selectorStart > f->frames()) edit->_selectorStart = 0;
                if(edit->_selectorEnd > f->frames()) edit->_selectorEnd = 0;
                
                edit->redraw();
            });
        };
        slr::EmitAction(std::move(act));
        return true;
    });
    
    _btnSilence = std::make_unique<Button>(this, "Silence");
    _btnSilence->setPos((Layout::Button*5) + (30*5), 0);
    _btnSilence->tapCallback([&edit = _editor](const GestLib::TapGesture &tap) -> bool {
        auto act = std::make_unique<slr::Actions::SilenceAudio>();
        act->start = edit->_selectorStart;
        act->end = edit->_selectorEnd;
        act->fileId = edit->_content.fileId;
        act->finished = [edit]() {
            UIControls::DisplayTask([edit]() {
                const slr::File *f = slr::ControlEngine::fileWorker()->findFileById(edit->_content.fileId);
                edit->_editor._fileLength = f->frames();
                if(edit->_editor._drawEnd > f->frames()) edit->_editor._drawEnd = f->frames();
                if(edit->_selectorStart > f->frames()) edit->_selectorStart = 0;
                if(edit->_selectorEnd > f->frames()) edit->_selectorEnd = 0;
                
                edit->redraw();
            });
        };
        slr::EmitAction(std::move(act));
        return true;
    });

    _btnReverse = std::make_unique<Button>(this, "Reverse");
    _btnReverse->setPos((Layout::Button*6) + (30*6), 0);
    _btnReverse->tapCallback([&edit = _editor](const GestLib::TapGesture &tap) -> bool {
        auto act = std::make_unique<slr::Actions::ReverseAudio>();
        act->start = edit->_selectorStart;
        act->end = edit->_selectorEnd;
        act->fileId = edit->_content.fileId;
        act->finished = [edit]() {
            UIControls::DisplayTask([edit]() {
                const slr::File *f = slr::ControlEngine::fileWorker()->findFileById(edit->_content.fileId);
                edit->_editor._fileLength = f->frames();
                if(edit->_editor._drawEnd > f->frames()) edit->_editor._drawEnd = f->frames();
                if(edit->_selectorStart > f->frames()) edit->_selectorStart = 0;
                if(edit->_selectorEnd > f->frames()) edit->_selectorEnd = 0;
                
                edit->redraw();
            });
        };
        slr::EmitAction(std::move(act));
        return true;
    });

    _btnReversePhase = std::make_unique<Button>(this, "Phase");
    _btnReversePhase->setPos((Layout::Button*7) + (30*7), 0);
    _btnReversePhase->tapCallback([&edit = _editor](const GestLib::TapGesture &tap) -> bool {

        return true;
    });

    
    _lblStaticStart = std::make_unique<Label>(this, "Start");
    _lblStaticStart->setPos(950, 20);
    _lblStaticStart->setSize(100, 40);
    _lblStart = std::make_unique<Label>(this, "0");
    _lblStart->setPos(1050, 20);
    _lblStart->setSize(300, 40);
    _lblStaticEnd = std::make_unique<Label>(this, "End");
    _lblStaticEnd->setPos(1200, 20);
    _lblStaticEnd->setSize(100, 40);
    _lblEnd = std::make_unique<Label>(this, "nan");
    _lblEnd->setPos(1300, 20);
    _lblEnd->setSize(300, 40);
    
    _btnRedraw = std::make_unique<Button>(this, "Redraw");
    _btnRedraw->setPos(Layout::WORKSPACE_WIDTH - ((Layout::Button*3) + (60)), 0);
    _btnRedraw->tapCallback([&edit = _editor](const GestLib::TapGesture &tap) -> bool {
        edit->redraw();
        return true;
    });


    _btnZoomTo = std::make_unique<Button>(this, "ZTo");
    _btnZoomTo->setPos(Layout::WORKSPACE_WIDTH - ((Layout::Button*2) + (40)), 0);
    _btnZoomTo->tapCallback([&edit = _editor](const GestLib::TapGesture &tap) -> bool {
        edit->_editor._drawStart = edit->_selectorStart;
        edit->_editor._drawEnd = edit->_selectorEnd;

        edit->_editor.calcNewZoom(edit->_editor._drawEnd - edit->_editor._drawStart);
        edit->_editor._zoomRedraw = true;
        return true;
    });

    _btnZoomOut = std::make_unique<Button>(this, "ZOut");
    _btnZoomOut->setPos(Layout::WORKSPACE_WIDTH - ((Layout::Button) + (20)), 0);
    _btnZoomOut->tapCallback([&edit = _editor](const GestLib::TapGesture &tap) -> bool {
        edit->_editor._drawStart = 0;
        edit->_editor._drawEnd = edit->_editor._fileLength;

        edit->_editor.calcNewZoom(edit->_editor._drawEnd);
        edit->_editor._zoomRedraw = true;
        return true;
    });
}

MidiToolbox::MidiToolbox(BaseWidget *parent/*, MidiEditor &editor*/) :
    BaseWidget(parent, true)
{
    setPos(0, 0);
    setSize(Layout::WORKSPACE_WIDTH, ToolboxHeight);
    noScroll();
    addStyle(&Style::workspace);

    _btnSnapToGrid = std::make_unique<Button>(this, "Snap");
    _btnSnapToGrid->setPos(50, 0);
    _btnSnapToGrid->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        if(this->_snapToGrid) this->_snapToGrid = false;
        else this->_snapToGrid = true;
        return true;
    });
    _btnSnapToGrid->touchUpCallback([this](const GestLib::TouchUpEvent &td) -> bool {
        if(!this->_snapToGrid) this->_btnSnapToGrid->setColor(Colors::ButtonReleased);
        else this->_btnSnapToGrid->setColor(Colors::ButtonPressed);
        return true;
    });

    _btnSelectTool = std::make_unique<Button>(this, "Select");
    _btnSelectTool->setPos(500, 0);
    _btnSelectTool->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        this->_lastTool = MidiTool::Select;
        return true;
    });
    _btnSelectTool->touchUpCallback([this](const GestLib::TouchUpEvent &td) -> bool {
        if(this->_lastTool == MidiTool::Select) {
            this->_btnSelectTool->setColor(Colors::ButtonPressed);
            this->_btnDrawTool->setColor(Colors::ButtonReleased);
            this->_btnEraseTool->setColor(Colors::ButtonReleased);
        }
        return true;
    });
    _btnSelectTool->setColor(Colors::ButtonPressed);

    _btnDrawTool = std::make_unique<Button>(this, "Draw");
    _btnDrawTool->setPos(600, 0);
    _btnDrawTool->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        this->_lastTool = MidiTool::Draw;
        return true;
    });
    _btnDrawTool->touchUpCallback([this](const GestLib::TouchUpEvent &td) -> bool {
        if(this->_lastTool == MidiTool::Draw) {
            this->_btnSelectTool->setColor(Colors::ButtonReleased);
            this->_btnDrawTool->setColor(Colors::ButtonPressed);
            this->_btnEraseTool->setColor(Colors::ButtonReleased);
        }
        return true;
    });

    _btnEraseTool = std::make_unique<Button>(this, "Erase");
    _btnEraseTool->setPos(700, 0);
    _btnEraseTool->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        this->_lastTool = MidiTool::Erase;
        return true;
    });
    _btnEraseTool->touchUpCallback([this](const GestLib::TouchUpEvent &td) -> bool {
        if(this->_lastTool == MidiTool::Erase) {
            this->_btnSelectTool->setColor(Colors::ButtonReleased);
            this->_btnDrawTool->setColor(Colors::ButtonReleased);
            this->_btnEraseTool->setColor(Colors::ButtonPressed);
        }
        return true;
    });
}

MidiEditor::MidiEditor(BaseWidget *parent, EditorView &editor, MidiToolbox &toolbox) :
    BaseWidget(parent, true),
    _editor(editor),
    _toolbox(toolbox)
{    
    setPos(Layout::WORKSPACE_POSITION_X, DrawableY);
    setSize(Layout::WORKSPACE_WIDTH, DrawableHeight); 
    noScroll();
    addStyle(&Style::workspace);

    lv_style_init(&styleDark);
    lv_style_copy(&styleDark, &Style::borderless);
    lv_style_set_bg_color(&styleDark, lv_color_hex(0x737373));
    lv_style_set_border_width(&styleDark, 1);
    lv_style_set_border_color(&styleDark, Colors::Black);
    
    lv_style_init(&styleBright);
    lv_style_copy(&styleBright, &Style::borderless);
    lv_style_set_bg_color(&styleBright, lv_color_darken(lv_color_hex(0xd9d9d9), 70));
    lv_style_set_border_width(&styleBright, 1);
    lv_style_set_border_color(&styleBright, Colors::Black);

    lv_style_init(&noteStyle);
    lv_style_copy(&noteStyle, &Style::borderless);
    lv_style_set_radius(&noteStyle, 5);
    lv_style_set_bg_color(&noteStyle, Colors::Black);

    _bglines.reserve(128);
    for(int i=0; i<128; ++i) {
        std::unique_ptr<Line> l = std::make_unique<Line>(this);

        l->hide();
        _bglines.push_back(std::move(l));
    }

    tapCallback(std::bind(&MidiEditor::handleTap, this, std::placeholders::_1));
    dragCallback(std::bind(&MidiEditor::handleDrag, this, std::placeholders::_1));
    show();
}

void MidiEditor::redraw() {
    notesDraw();
}

void MidiEditor::preparePreview(const slr::MidiFile *file) {
    slr::TimelineView &tl = slr::TimelineView::getTimelineView();
    const slr::MidiPeaks *peaks = file->peaks(0);
    const std::vector<slr::Note> &notes = peaks->notes();
    _file = file;
    
    //set editor vars...
    _editor._drawStart = 0;
    _editor._drawEnd = (peaks->notes().back().ppqnEnd / (float)tl.ppqn()) * tl.framesPerQuater();
    _editor._fileLength = _editor._drawEnd;

    _noteMin = 127;
    _noteMax = 0;
    //get range 
    for(const auto &n : notes) {
        _noteMin = Math::min(_noteMin, n.note);
        _noteMax = Math::max(_noteMax, n.note);
    }

    if(_noteMin > _noteMax) {
        LOG_ERROR("Some error appeared");
        return;
    }
    
    _fNoteMin = _noteMin;
    _fNoteMax = _noteMax;
    
    notesDraw();
}

void MidiEditor::notesDraw() {
    slr::TimelineView &tl = slr::TimelineView::getTimelineView();
    for(auto &line : _bglines) line->hide();

    int notesOnDisplay = (_noteMax - _noteMin)+1;
    int totalHeight = DrawableHeight;
    float heightPerNote = static_cast<float>(totalHeight) / notesOnDisplay;
    int h = Math::round(heightPerNote);

    _heightPerRow = h;
    _rowsOnDisplay = notesOnDisplay;

    for(int n=0; n<notesOnDisplay; ++n) {
        Line *l = _bglines[n].get();

        int xpos = 0;
        int ypos = Math::round(n * heightPerNote);
    
        l->setSize(Layout::WORKSPACE_WIDTH, h);
        l->setPos(xpos, ypos);
        l->addStyle( blackKeys[(_noteMax - n)%12] ? &styleDark : &styleBright );

        if(_heightPerRow > 6) {
            l->_noteText = lv_label_create(l->lvhost());
            std::string text = std::to_string(_noteMax-n);
            lv_label_set_text(l->_noteText, text.c_str());
            const lv_font_t * font = pickFont(_heightPerRow);
            lv_obj_set_style_text_font(l->_noteText, font, 0);
            lv_obj_set_style_text_color(l->_noteText, Colors::White, 0);
            lv_obj_set_pos(l->_noteText, 10, (_heightPerRow/2) - (lv_font_get_line_height(font)/2));
            // lv_obj_set_size(l->_noteText, 20, _heightPerRow);
        }

        l->show();
    }

    _notePreviews.clear();
    
    const std::vector<slr::Note> &notes = _file->peaks(0)->notes();
    const double pixelsPerTick = static_cast<double>(Layout::WORKSPACE_WIDTH) / static_cast<double>(_editor._drawEnd - _editor._drawStart);
    for(const auto &n : notes) {
        if(n.note < _noteMin && n.note > _noteMax) continue;
        slr::frame_t start = UIUtility::ppqnToFrames(n.ppqnStart, tl);
        slr::frame_t end = UIUtility::ppqnToFrames(n.ppqnEnd, tl);
        if(start >= _editor._drawEnd || end <= _editor._drawStart) continue;
        
        start = Math::max(start, _editor._drawStart) - _editor._drawStart;
        end = Math::min(end, _editor._drawEnd) - _editor._drawStart;

        int xpos = Math::round(start * pixelsPerTick);
        int w = Math::round((end-start) * pixelsPerTick);
        int ypos = Math::floor((_noteMax - n.note) * heightPerNote);

        std::unique_ptr<NotePreview> nprev = std::make_unique<NotePreview>(this, *this, n);
        nprev->x = xpos;
        nprev->y = ypos;
        nprev->w = w;
        nprev->h = Math::round(heightPerNote);

        nprev->setPos(xpos, ypos);
        nprev->setSize(w, nprev->h);
        nprev->show();
        _notePreviews.push_back(std::move(nprev));
    }
}

int MidiEditor::pitchAtY(int y) {
    return _noteMax - ((y-(Layout::TOP_PANEL_HEIGHT+DrawableY)) / _heightPerRow)+1;
}

int MidiEditor::yToNoteY(int y) {
    int possiblePitch = pitchAtY(y);
    return Math::floor((_noteMax - possiblePitch) * _heightPerRow);
}

bool MidiEditor::handleTap(const GestLib::TapGesture &tap) {
   switch(_toolbox.lastTool()) {
        case(MidiTool::Select): {
            //select many notes
        } break;
        case(MidiTool::Draw): {
            //add new note within grid
            UI::Timeline *tl = _editor._timeline.get();
            //there is possibility that after zoom we didn't updated timeline yet, but problem exists within 17ms time frame soo...
            std::pair<int, int> pos = tl->positionsWithinGrid(tap.x);
            //add drawable at pos
            
            const slr::TimelineView &tlv = slr::TimelineView::getTimelineView();

            slr::Note n;
            n.note = pitchAtY(tap.y);
            n.velocity = 100;
            n.channel = 0;
            //need to quantize this?
            n.ppqnStart = UIUtility::framesToPpqn(_editor._timeline->positionToFrames(pos.first), tlv);
            n.ppqnEnd = UIUtility::framesToPpqn(_editor._timeline->positionToFrames(pos.first+pos.second), tlv);

            LOG_INFO("Adding new Midi note");
            auto act = std::make_unique<slr::Actions::AddNewMidiNote>();
            act->note.note = n.note;
            act->note.velocity = n.velocity;
            act->note.channel = n.channel;
            act->note.ppqnStart = n.ppqnStart;
            act->note.ppqnEnd = n.ppqnEnd;
            act->fileId = _file->id();
            act->midiTrackId = 0;
            slr::EmitAction(std::move(act));

            std::unique_ptr<NotePreview> p = std::make_unique<NotePreview>(this, *this, n);
            p->x = pos.first;
            p->y = yToNoteY(tap.y);
            p->h = _heightPerRow;
            p->w = pos.second;
            p->_changingLength = false;
            p->_changingStart = false;
            p->setPos(p->x, p->y);
            p->setSize(p->w, p->h);
            p->show();

            _notePreviews.push_back(std::move(p));
        } break;
        case(MidiTool::Erase): {
            //erase note under tap coords if exists
            auto it = std::find_if(
                _notePreviews.begin(),
                _notePreviews.end(),
                [tap](const std::unique_ptr<NotePreview> &n) {
                    return UIUtility::withinRange(tap.x, n->x, n->x+n->w) && 
                            UIUtility::withinRange(tap.y-(Layout::TOP_PANEL_HEIGHT + DrawableY), n->y, n->y+n->h);
                }
            );

            if(it != _notePreviews.end()) {
                // int pitch = pitchAtY((*it)->y);
                // // slr::frame_t ppqnStart = UIUtility::framesToPpqn(_editor._timeline->positionToFrames((*it)->x));
                // // slr::frame_t ppqnEnd = UIUtility::framesToPpqn(_editor._timeline->positionToFrames((*it)->x+(*it)->w));
                // int velocity = 100;
                // int channel = 0;
                
                auto act = std::make_unique<slr::Actions::EraseMidiNotes>();
                slr::Note n = (*it)->_note;
                act->notes.push_back(n);
                act->fileId = _file->id();
                act->midiTrackId = 0;
                
                slr::EmitAction(std::move(act));

                LOG_WARN("[Placeholder] Erase note");
                
                _notePreviews.erase(it);
            }

        } break;
   }
    return true;
}

bool MidiEditor::handleDrag(const GestLib::DragGesture &drag) {
    /*if(_toolbox.isSelectTool()) {
        //use region to select many notes so later all can be moved?
    } else if(_toolbox.isDrawTool()) {
        //draw new note, drag from end(as coded down below)
    } else if(_toolbox.isEraseTool()) {
        //...??erase everything within selected region?
    }*/

    switch(_toolbox.lastTool()) {
        case(MidiTool::Select): {

        } break;
        case(MidiTool::Draw): {

        if(drag.state == GestLib::GestureState::Start) {
            
            // int possiblePitch = _noteMax - ((drag.y-(Layout::TOP_PANEL_HEIGHT+DrawableY)) / _heightPerRow)+1;
            // int y =  Math::floor((_noteMax - possiblePitch) * _heightPerRow);

            slr::Note n;
            n.note = pitchAtY(drag.y);
            n.velocity = 100;
            n.channel = 0;
            n.ppqnStart = 0;
            n.ppqnEnd = 0;

            std::unique_ptr<NotePreview> p = std::make_unique<NotePreview>(this, *this, n);
            p->x = drag.x-30;
            p->y = yToNoteY(drag.y);
            p->h = _heightPerRow;
            p->w = 30;
            p->_changingLength = true;
            p->_changingStart = false;
            p->setPos(p->x, p->y);
            p->setSize(p->w, p->h);
            p->show();

            _notePreviews.push_back(std::move(p));
            _draggableNote = &_notePreviews.back(); //std::prev(_notePreviews.end());
            // (*_draggableNote)->handleDrag(drag);
            // _editor._uictx->transferGesture(_notePreviews.back().get(), GestLib::Gestures::Drag);
        } else if(drag.state == GestLib::GestureState::Move) {
            if(_draggableNote) (*_draggableNote)->handleDrag(drag);
        } else if(drag.state == GestLib::GestureState::End) {
            //recreate 
            if(_draggableNote) {
                (*_draggableNote)->handleDrag(drag);

                const slr::TimelineView &tlv = slr::TimelineView::getTimelineView();

                slr::Note n;
                n.note = (*_draggableNote)->_note.note;
                n.velocity = (*_draggableNote)->_note.velocity;
                n.channel = (*_draggableNote)->_note.channel;
                n.ppqnStart = UIUtility::framesToPpqn(_editor._timeline->positionToFrames((*_draggableNote)->x), tlv);
                n.ppqnEnd = UIUtility::framesToPpqn(_editor._timeline->positionToFrames((*_draggableNote)->x+(*_draggableNote)->w), tlv);

                LOG_INFO("Adding new Midi note");

                auto act = std::make_unique<slr::Actions::AddNewMidiNote>();
                act->note.note = n.note;
                act->note.velocity = n.velocity;
                act->note.channel = n.channel;
                act->note.ppqnStart = n.ppqnStart;
                act->note.ppqnEnd = n.ppqnEnd;
                act->fileId = _file->id();
                act->midiTrackId = 0;
                slr::EmitAction(std::move(act));

                std::unique_ptr<NotePreview> p = std::make_unique<NotePreview>(this, *this, n);
                p->x = (*_draggableNote)->x;
                p->y = (*_draggableNote)->y;
                p->h = (*_draggableNote)->h;
                p->w = (*_draggableNote)->w;
                p->_changingLength = false;
                p->_changingStart = false;
                p->setPos(p->x, p->y);
                p->setSize(p->w, p->h);
                p->show();
                
                _draggableNote->swap(p); //old get's deleted
                _draggableNote = nullptr;
            }
        }
        } break;
        case(MidiTool::Erase): {
            //TODO: use an selector area to select many notes at once and erase them
        } break;
    }

    return true;
}

NotePreview::NotePreview(BaseWidget *parent, MidiEditor &editor, const slr::Note n) :
    BaseWidget(parent, true),
    _editor(editor),
    _note(n)
{
    addStyle(&noteStyle);
    noScroll();

    holdCallback(std::bind(&NotePreview::handleHold, this, std::placeholders::_1));
    dragCallback(std::bind(&NotePreview::handleDrag, this, std::placeholders::_1));
    swipeCallback(std::bind(&NotePreview::handleSwipe, this, std::placeholders::_1));
}

bool NotePreview::handleHold(const GestLib::HoldGesture &hold) {
    moveNote(hold.x, hold.y, hold.state == GestLib::GestureState::End ? true : false);
    return true;
}

bool NotePreview::handleDrag(const GestLib::DragGesture &drag) {
    // moveNote(drag.x, drag.y, drag.state == GestLib::GestureState::End ? true : false);
    if(drag.state == GestLib::GestureState::Start) {
        //check if the gesture is within... 30px of note edge
        if(UIUtility::withinRange(drag.x, x, x+30) ||
            UIUtility::withinRange(drag.x, x+w-30, x+w))
        {
            _changingLength = true;
        } else {
            _changingLength = false;
        }

        if(UIUtility::withinRange(drag.x, x, x+30)) _changingStart = true;
        else _changingStart = false;

        return _changingLength;
    } else if(drag.state == GestLib::GestureState::Move) {
        if(!_changingLength) return false;

        if(_changingStart) {
            int oldx = x;
            if(w + (oldx-drag.x) > 30) {
                w = w + (oldx-drag.x);
                x = drag.x;
            }
            
        } else {
            w = drag.x - x;
            if(w < 30) w = 30;
        }

        setPos(x, y);
        setSize(w, h);
        return true;
    } else if(drag.state == GestLib::GestureState::End) {
        //TODO: set final and emit action
        if(!_changingLength) return false; //ignore

        if(_changingStart) {
            int oldx = x;
            if(w + (oldx-drag.x) > 30) {
                w = w + (oldx-drag.x);
                x = drag.x;
            }
            
            LOG_WARN("[Placeholder] change note start");
        } else {
            w = drag.x - x;
            if(w < 30) w = 30;
            
            LOG_WARN("[Placeholder] change note length");
        }

        setPos(x, y);
        setSize(w, h);
        return true;
    }

    return true;
}

bool NotePreview::handleSwipe(const GestLib::SwipeGesture &swipe) {
    return true;
}

void NotePreview::moveNote(int newx, int newy, bool isEnd) {
    x = newx;
    y = newy - (Layout::TOP_PANEL_HEIGHT + DrawableY);
    y -= h/2;
    x -= w/2;

    
    if(isEnd) {
        int pitch = _editor.pitchAtY(newy);
        y = _editor.yToNoteY(newy);
        // y = Math::round(_editor._noteMax - (possiblePitch - _editor._noteMin));

        // slr::frame_t aproxStart = newx;
        // slr::frame_t aproxEnd = newx + w;
        //need to mul start and end to a framesPerPixel value
        
        LOG_INFO("Possible note %d", pitch);
    }
    
    setPos(x, y);
}



}
