// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "display/primitives/BaseWidget.h"
#include "snapshots/ClipContainerView.h"
#include "common/defines.h"

#include <vector>
#include <memory>

namespace slr {
    struct MidiFile;
    struct AudioFile;
}

namespace UI {
struct UIContext;
struct Timeline;
struct Label;
struct FilePreview;
struct AudioEditor;
struct MidiEditor;

struct EditorView : public BaseWidget {
    EditorView(BaseWidget * parent, UIContext * const uictx);
    ~EditorView();

    void pollUIUpdate() override;

    void setEditable(slr::ID fileId);

    private:
    UIContext * const _uictx;

    lv_obj_t * _toolPanel;
    std::unique_ptr<Label> _lblFilePath;
    slr::frame_t _fileLength;
    
    bool _zoomRedraw;
    double _horizontalZoom;
    slr::frame_t _drawStart;
    slr::frame_t _drawEnd;

    std::unique_ptr<Timeline> _timeline;

    std::unique_ptr<AudioEditor> _audioEditor;
    std::unique_ptr<MidiEditor> _midiEditor;

    bool _dragStartedHere;

    bool handleSwipe(const GestLib::SwipeGesture & swipe);
    bool handleDrag(const GestLib::DragGesture &drag);
    bool handleZoom(const GestLib::ZoomGesture &zoom);

    friend class AudioEditor;
    friend class MidiEditor;
};


struct AudioEditor : public BaseWidget {
    AudioEditor(BaseWidget *, EditorView &editor);

    void redraw();
    void preparePreview(const slr::AudioFile*);

    slr::ClipContentView _content;
    std::unique_ptr<FilePreview> _preview;

    private:
    EditorView & _editor;
    // bool handleHold(const GestLib::HoldGesture &);
};

struct Line : public BaseWidget {
    Line(BaseWidget *parent) : BaseWidget(parent, true, false) {}
    ~Line() { if(_noteText) lv_obj_delete(_noteText); }

    void hide() override { BaseWidget::hide(); if(_noteText) lv_obj_add_flag(_noteText, LV_OBJ_FLAG_HIDDEN); }
    void show() override { BaseWidget::show(); if(_noteText) lv_obj_clear_flag(_noteText, LV_OBJ_FLAG_HIDDEN); }

    lv_obj_t * _noteText = nullptr;
    // lv_obj_t * bg; //different for white and black keys... just lv_obj_create() with different colors
};

struct MidiEditor;
struct NotePreview : public BaseWidget {
    NotePreview(BaseWidget *, MidiEditor&);

    int x;
    int y;
    int w; 
    int h;

    bool _changingLength;
    bool _changingStart;
    
    private:
    MidiEditor &_editor;

    
    bool handleHold(const GestLib::HoldGesture &);
    bool handleDrag(const GestLib::DragGesture &);
    bool handleSwipe(const GestLib::SwipeGesture &);

    void moveNote(int newx, int newy, bool isEnd);
};
    
struct MidiEditor : public BaseWidget {
    MidiEditor(BaseWidget *parent, EditorView &editor);

    void redraw();
    void preparePreview(const slr::MidiFile*);
    void notesDraw();

    std::vector<std::unique_ptr<Line>> _bglines;
        
    std::vector<std::unique_ptr<NotePreview>> _notePreviews;
    float _fNoteMin;
    float _fNoteMax;
    int _noteMin;
    int _noteMax;
    int _rowsOnDisplay;
    float _heightPerRow;
    double _verticalZoom;

    // std::vector<std::unique_ptr<Line>> _lines;
    //draw, like... vertical lines, tooling, etc, dunno...

    private:
    const slr::MidiFile *_file;
    EditorView &_editor;
};

/*
struct MidiEditorToolbox {

};

drag or swipe to change length, 
hold for pick and move around

*/
}