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

struct NotePreview {
    int x;
    int y;
    int w; 
    int h;
};

struct EditorView : public BaseWidget {
    EditorView(BaseWidget * parent, UIContext * const uictx);
    ~EditorView();

    float hZoom() const { return _horizontalZoom; }
    float vZoom() const { return _verticalZoom; }

    void pollUIUpdate() override;

    void setEditable(slr::ID fileId);

    private:
    UIContext * const _uictx;

    lv_obj_t * _toolPanel;
    std::unique_ptr<Label> _lblFilePath;
    
    float _horizontalZoom;
    float _verticalZoom;

    std::unique_ptr<Timeline> _timeline;
    slr::ClipContentView _content;
    std::unique_ptr<FilePreview> _preview;

    struct AudioEditor {

    };

    struct Line {
        ~Line() { lv_obj_delete(line); lv_obj_delete(num); }
        lv_obj_t *line;
        lv_obj_t *num;
        lv_point_precise_t position[2];

        void show() { lv_obj_remove_flag(line, LV_OBJ_FLAG_HIDDEN); lv_obj_remove_flag(num, LV_OBJ_FLAG_HIDDEN); }
        void hide() { lv_obj_add_flag(line, LV_OBJ_FLAG_HIDDEN); lv_obj_add_flag(num, LV_OBJ_FLAG_HIDDEN); }
    };

    struct MidiEditor : public BaseWidget {
        MidiEditor(BaseWidget *parent);
        std::vector<std::unique_ptr<Line>> _lines;
        //draw, like... vertical lines, tooling, etc, dunno...

        std::vector<NotePreview> _notePreviews;
    };

    MidiEditor _midiEditor;

    bool _dragStartedHere;

    bool handleSwipe(const GestLib::SwipeGesture & swipe);
    bool handleDrag(const GestLib::DragGesture &drag);
    bool handleZoom(const GestLib::ZoomGesture &zoom);
};

}