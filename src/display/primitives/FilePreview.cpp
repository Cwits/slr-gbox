// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display/primitives/FilePreview.h"

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
#include "snapshots/AudioUnitView.h"
#include "snapshots/ClipContainerView.h"

#include "display/primitives/Button.h"
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

FilePreview::FilePreview(BaseWidget * parent, const slr::ClipContentView * const clipItem, UIContext * const uictx) :
    BaseWidget(parent, true, true),
    _uictx(uictx),
    _content(clipItem)
{
    addStyle(&Style::GridFile);
    // lv_obj_set_style_border_width(lvhost(), 2, 0);
    // lv_obj_set_style_border_color(lvhost(), Colors::White, 0);
    noScroll();
    _canvas = lv_canvas_create(lvhost());
    lv_obj_set_pos(_canvas, 0, 0);

    _canvasWidth = 0;
    _canvasHeight = 0;
}

FilePreview::~FilePreview() {
    lv_obj_delete(_canvas);
}

slr::ID FilePreview::id() const {
    return _content->fileId;
}

void FilePreview::setSize(lv_coord_t w, lv_coord_t h) {
    BaseWidget::setSize(w, h);
    _canvasWidth = w;
    _canvasHeight = h;
    
    //re?allocate buffer
    if(_drawBuffer) _drawBuffer.reset();
    _drawBuffer = std::make_unique<uint8_t[]>(
        LV_DRAW_BUF_SIZE(
            _canvasWidth, 
            _canvasHeight, 
            LV_COLOR_FORMAT_ARGB8888
        )
    );
    lv_canvas_set_buffer(_canvas, _drawBuffer.get(), _canvasWidth, _canvasHeight, LV_COLOR_FORMAT_ARGB8888);
}

void FilePreview::draw(slr::frame_t frameStart, slr::frame_t frameEnd, int notefrom, int noteto) {
    lv_canvas_fill_bg(_canvas, Colors::clipPreviewBackground, LV_OPA_30);

    if(_content->isAudio()) {        
        UIHelpers::audioFileToCanvas(
            _content->audio,
            frameStart,
            frameEnd,
            _canvas,
            _canvasHeight,
            _canvasWidth,
            _peakColor,
            _fillColor
        );
    } else if(_content->isMidi()) {
        UIHelpers::midiFileToCanvas(
            _content->midi, 
            frameStart, 
            frameEnd,
            notefrom,
            noteto,
            _canvas,
            _canvasHeight,
            _canvasWidth,
            _peakColor,
            _fillColor
        );
    }
}

void FilePreview::pollUIUpdate() {

}




}

