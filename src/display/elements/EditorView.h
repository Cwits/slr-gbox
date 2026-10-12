// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "display/primitives/BaseWidget.h"
#include "snapshots/FileContainerView.h"
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

    struct MidiEditor {

    };

    bool handleSwipe(const GestLib::SwipeGesture & swipe);
    bool handleDrag(const GestLib::DragGesture &drag);
    bool handleZoom(const GestLib::ZoomGesture &zoom);
};

}