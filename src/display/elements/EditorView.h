// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "display/primitives/BaseWidget.h"
#include "snapshots/ClipContainerView.h"
#include "core/primitives/MidiPeaks.h"
#include "common/defines.h"

#include <vector>
#include <memory>

namespace slr {
    struct MidiFile;
    struct AudioFile;
}

namespace UI {
struct Button;
struct UIContext;
struct Timeline;
struct Label;
struct FilePreview;
struct AudioEditor;
struct AudioToolbox;
struct MidiEditor;
struct MidiToolbox;

struct EditorView : public BaseWidget {
    EditorView(BaseWidget * parent, UIContext * const uictx);
    ~EditorView();

    void pollUIUpdate() override;

    void setEditable(slr::ID fileId);
    void calcNewZoom(slr::frame_t size);
    
    private:
    UIContext * const _uictx;

    std::unique_ptr<Label> _lblFilePath;
    
    slr::frame_t _fileLength;
    
    bool _zoomRedraw;
    slr::frame_t _drawStart;
    slr::frame_t _drawEnd;
    
    double _horizontalZoom;
    std::unique_ptr<Timeline> _timeline;

    std::unique_ptr<AudioEditor> _audioEditor;
    std::unique_ptr<AudioToolbox> _audioToolbox;
    std::unique_ptr<MidiEditor> _midiEditor;
    std::unique_ptr<MidiToolbox> _midiToolbox;

    bool handleSwipe(const GestLib::SwipeGesture & swipe);
    bool handleDrag(const GestLib::DragGesture &drag);
    bool handleZoom(const GestLib::ZoomGesture &zoom);

    friend class AudioEditor;
    friend class AudioToolbox;
    friend class MidiEditor;
};


struct AudioEditor : public BaseWidget {
    AudioEditor(BaseWidget *, EditorView &editor, AudioToolbox &);
    ~AudioEditor();

    void redraw();
    void preparePreview(const slr::AudioFile*);

    slr::ClipContentView _content;
    std::unique_ptr<FilePreview> _preview;

    slr::frame_t _selectorStart;
    slr::frame_t _selectorEnd;

    void showHandles();
    void hideHandles();

    private:
    EditorView & _editor;
    AudioToolbox &_toolbox;
    
    bool handleDrag(const GestLib::DragGesture &); //allows to draw staer and end

    struct Handle : public BaseWidget {
        Handle(BaseWidget *parent);


        bool handleDrag(const GestLib::DragGesture &);
        AudioEditor *_editor;
        bool _endHandle = false;
    };

    
    lv_obj_t * _filler;
    lv_obj_t * _lineStart;
    lv_point_precise_t _pointsStart[2];

    lv_obj_t * _lineEnd;
    lv_point_precise_t _pointsEnd[2];

    std::unique_ptr<Handle> _handleStart;
    std::unique_ptr<Handle> _handleEnd;

    friend class AudioToolbox;
};

struct AudioToolbox : public BaseWidget {
    AudioToolbox(BaseWidget *parent);

    private:
    AudioEditor *_editor;

    std::unique_ptr<Button> _btnSelectAll;
    std::unique_ptr<Button> _btnCut; //+
    std::unique_ptr<Button> _btnCopy; //+
    std::unique_ptr<Button> _btnPaste;//+
    std::unique_ptr<Button> _btnDelete; //+
    std::unique_ptr<Button> _btnSilence; //+
    std::unique_ptr<Button> _btnReverse; 
    std::unique_ptr<Button> _btnReversePhase;

    std::unique_ptr<Label> _lblStaticStart;
    std::unique_ptr<Label> _lblStart;
    std::unique_ptr<Label> _lblStaticEnd;
    std::unique_ptr<Label> _lblEnd;

    std::unique_ptr<Button> _btnRedraw;
    std::unique_ptr<Button> _btnZoomTo;
    std::unique_ptr<Button> _btnZoomOut;

    friend class EditorView;
    friend class AudioEditor;
};

struct Line : public BaseWidget {
    Line(BaseWidget *parent) : BaseWidget(parent, true, false) {}
    ~Line() { if(_noteText) lv_obj_delete(_noteText); }

    void hide() override { BaseWidget::hide(); if(_noteText) lv_obj_add_flag(_noteText, LV_OBJ_FLAG_HIDDEN); }
    void show() override { BaseWidget::show(); if(_noteText) lv_obj_clear_flag(_noteText, LV_OBJ_FLAG_HIDDEN); }

    lv_obj_t * _noteText = nullptr;
    // lv_obj_t * bg; //different for white and black keys... just lv_obj_create() with different colors
};

struct NotePreview;
    
struct MidiEditor : public BaseWidget {
    MidiEditor(BaseWidget *parent, EditorView &editor, MidiToolbox &toolbox);

    void redraw();
    void preparePreview(const slr::MidiFile*);
    void notesDraw();

    int pitchAtY(int y);
    int yToNoteY(int y);

    std::vector<std::unique_ptr<Line>> _bglines;
        
    std::vector<std::unique_ptr<NotePreview>> _notePreviews;
    float _fNoteMin;
    float _fNoteMax;
    int _noteMin;
    int _noteMax;
    int _rowsOnDisplay;
    float _heightPerRow;
    double _verticalZoom;

    private:
    const slr::MidiFile *_file;
    EditorView &_editor;
    MidiToolbox &_toolbox;

    std::unique_ptr<NotePreview>* _draggableNote = nullptr;

    bool handleTap(const GestLib::TapGesture &);
    bool handleDrag(const GestLib::DragGesture &); //guess this must be tool context dependent
};

enum class MidiTool {
    Select,
    Draw,
    Erase
}; 

struct MidiToolbox : public BaseWidget {
    MidiToolbox(BaseWidget *parent/*, MidiEditor &editor*/);
    
    bool snapToGrid() const { return _snapToGrid; }
    MidiTool lastTool() const { return _lastTool; }

    private:
    bool _snapToGrid = false;
    MidiTool _lastTool = MidiTool::Select;

    std::unique_ptr<Button> _btnSnapToGrid;
    std::unique_ptr<Button> _btnSelectTool;
    std::unique_ptr<Button> _btnDrawTool;
    std::unique_ptr<Button> _btnEraseTool;
    
    // std::unique_ptr<Button> _btnZoomTo;
    // std::unique_ptr<Button> _btnZoomOut;
};

struct NotePreview : public BaseWidget {
    NotePreview(BaseWidget *, MidiEditor&, const slr::Note);

    int x;
    int y;
    int w; 
    int h;

    bool _changingLength;
    bool _changingStart;
    
    const slr::Note _note;

    
    bool handleHold(const GestLib::HoldGesture &);
    bool handleDrag(const GestLib::DragGesture &);
    bool handleSwipe(const GestLib::SwipeGesture &);

    private:
    MidiEditor &_editor;
    void moveNote(int newx, int newy, bool isEnd);
};
// drag or swipe to change length, 
// hold for pick and move around

}