// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/ModEngineView.h"

#include "ui/display/primitives/UIContext.h"
#include "ui/display/primitives/Label.h"
#include "ui/display/primitives/Button.h"
#include "ui/display/primitives/DropDown.h"

#include "ui/display/PopupManager.h"
#include "ui/display/layoutSizes.h"
#include "ui/display/defaultColors.h"
#include "ui/display/defaultStyles.h"

#include "ui/display/helpers/Macros.h"


#include "snapshots/TimelineView.h"
#include "core/Actions.h"
#include "ui/uiutility.h"

#include "common/Math.h"
#include "logger.h"

#include <cmath>

namespace UI {


void fillCanvas(lv_obj_t *cnv, int w, int h, lv_color_t clr) {
    for(int x=0; x<w; ++x) {
        for(int y=0; y<h; ++y) {
            lv_canvas_set_px(cnv, x, y, clr, LV_OPA_COVER);
        }
    }
}

ModEngineView::ModEngineView(BaseWidget * parent, UIContext * const uictx) :
    View(parent, uictx)
{
    setPos(LayoutDef::STEP_SEQ_X, LayoutDef::STEP_SEQ_Y);
    setSize(LayoutDef::STEP_SEQ_WIDTH, LayoutDef::STEP_SEQ_HEIGHT);
    setColor(lv_color_hex(0x06e17e));

    LABEL(_lblModNumText, LayoutDef::DEFAULT_MARGIN, LayoutDef::DEFAULT_MARGIN+10, 200, 40, "Modulation: ");
    LABEL(_lblModNum,     210,                       LayoutDef::DEFAULT_MARGIN+10, 100, 40, "x / x");
    
    BUTTON(_btnNextMod, 1500, LayoutDef::DEFAULT_MARGIN, LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE, LV_SYMBOL_LEFT);
    BUTTON(_btnPrevMod, 1600, LayoutDef::DEFAULT_MARGIN, LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE, LV_SYMBOL_RIGHT);
    BUTTON(_btnNewMod,  1700, LayoutDef::DEFAULT_MARGIN, LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE, LV_SYMBOL_PLUS);
    BUTTON(_btnDelMod,  1800, LayoutDef::DEFAULT_MARGIN, LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE, LV_SYMBOL_TRASH);

    int y = 120;
    LABEL(_lblTypeText, LayoutDef::DEFAULT_MARGIN, y, 100, 40, "Type: ");
    _ddType = std::make_unique<DropDown>(this);
    _ddType->setPos(110, y);
    _ddType->setSize(160, 40);
    std::vector<std::string> list;
    list.push_back("Sin"); list.push_back("Square"); list.push_back("Saw"); list.push_back("Tri");// list.push_back("Rand");
    _ddType->setItems(list);
    _ddType->setSelected("Sin");
    _ddType->selectedCallback([this](std::string selected) {
        this->redrawCanvas(selected, this->_tmpRate, this->_tmpOffset);
    });

    LABEL(_lblRateText, 300+50, y, 100, 40, "Rate: ");
    LABEL(_lblRate,     380+50, y, 100, 40, "1.00 Hz");
    _lblRate->setTapCallback([this]() {
        std::string initText = this->_lblRate->text().substr(0, this->_lblRate->text().size()-2);
        this->_uictx->_popManager->enableKeyboard(
            initText,
            [this](const std::string & text) {
                std::string toSet = text;
                toSet.append(" Hz");
                this->_lblRate->setText(toSet);
                this->_tmpRate = std::stof(text);
                this->redrawCanvas(this->_ddType->selectedItem(), this->_tmpRate, this->_tmpOffset);
            }
        );
    });
    _lblRate->setDragCallback([this](GestLib::DragGesture &drag) {
        static int slowMeDown = 0;
        slowMeDown++;
        if(drag.dx > 0) {
            if(this->_tmpRate < 10.f) {
                this->_tmpRate += 0.1f;
            } else if(this->_tmpRate < 1000.f) {
                this->_tmpRate += 1.f;
            }
            
            if(slowMeDown < 3) return;
            slowMeDown = 0;

            std::string rateStr = std::to_string(this->_tmpRate);
            rateStr.append(" Hz");
            this->_lblRate->setText(rateStr);
            redrawCanvas(this->_ddType->selectedItem(), this->_tmpRate, this->_tmpOffset);
        } else if(drag.dx < 0) {
            if(this->_tmpRate > 1000.f) {
                this->_tmpRate -= 1.f;
            } else if(this->_tmpRate > 0.f) {
                this->_tmpRate -= 0.1f;
            }
            
            if(slowMeDown < 3) return;
            slowMeDown = 0;

            std::string rateStr = std::to_string(this->_tmpRate);
            rateStr.append(" Hz");
            this->_lblRate->setText(rateStr);
            redrawCanvas(this->_ddType->selectedItem(), this->_tmpRate, this->_tmpOffset);
        }
    });

    BUTTON(_btnRate,    300+50, y+40, LayoutDef::BUTTON_SIZE+80, 40, "Hz/Beat");
    
    LABEL(_lblOffsetText, 600+50, y, 100, 40, "Offset: ");
    LABEL(_lblOffset,   710+50, y, 100, 40, "0.00");
    _lblOffset->setTapCallback([this]() {
        this->_uictx->_popManager->enableKeyboard(
            this->_lblOffset->text(),
            [this](const std::string & text) {
                this->_lblOffset->setText(text);
                this->_tmpOffset = std::stof(text);
                this->redrawCanvas(this->_ddType->selectedItem(), this->_tmpRate, this->_tmpOffset);
            }
        );
    });
    _lblOffset->setDragCallback([this](GestLib::DragGesture &drag) {
        static int slowMeDown = 0;
        slowMeDown++;
        if(drag.dx > 0) {
            if(this->_tmpOffset < 360.0f) {
                this->_tmpOffset += 0.01f;
            }
            
            if(slowMeDown < 3) return;
            slowMeDown = 0;

            std::string rateStr = std::to_string(this->_tmpOffset);
            // rateStr.append("");
            this->_lblOffset->setText(rateStr);
            redrawCanvas(this->_ddType->selectedItem(), this->_tmpRate, this->_tmpOffset);
        } else if(drag.dx < 0) {
            if(this->_tmpOffset > 0) {
                this->_tmpOffset -= 0.01f;
            }
            if(slowMeDown < 3) return;
            slowMeDown = 0;

            std::string rateStr = std::to_string(this->_tmpOffset);
            // rateStr.append(" Hz");
            this->_lblOffset->setText(rateStr);
            redrawCanvas(this->_ddType->selectedItem(), this->_tmpRate, this->_tmpOffset);
        }
    });

    // LABEL(_lblRandSeedText, 900+50, y, 100, 40, "Seed: ");
    // LABEL(_lblRandSeed,    1010+50, y, 100, 40, "0");
    // LABEL(_lblLengthText, 1200+50, y, 120, 40, "Length: ");
    // LABEL(_lblLength,   1200+140+50, y, 100, 40, "Err");

    // BUTTON(_btnLength,  1200+50, y+40, LayoutDef::BUTTON_SIZE+80, 40, "Sec/Beat");

    BUTTON(_btnManageTargets, 1700, y, LayoutDef::BUTTON_SIZE*2, LayoutDef::BUTTON_SIZE, "Targets");



    // _cnvModView = std::unique_ptr<lv_obj_t>(
    //     lv_canvas_create(lvhost()), 
    //     [](lv_obj_t * canvas) {
    //         lv_obj_delete(canvas);
    //     }
    // );
    
    // line &lin = _lines.back();
    _line = lv_line_create(lvhost());
    _points[0] = {360, 240};
    _points[1] = {360, 840};
    lv_line_set_points(_line, &_points[0], 2);
        
    lv_obj_add_style(_line, &gridLine, 0);

    _cnvModView = lv_canvas_create(lvhost());
    lv_obj_set_pos(_cnvModView, 360, 240);
    lv_obj_set_size(_cnvModView, 1200, 600);

    _drawBuffer = new uint8_t[LV_DRAW_BUF_SIZE(1200, 600, LV_COLOR_FORMAT_NATIVE)];
    lv_canvas_set_buffer(_cnvModView, _drawBuffer, 1200, 600, LV_COLOR_FORMAT_NATIVE);

    redrawCanvas(_ddType->selectedItem(), _tmpRate, _tmpOffset);
}

ModEngineView::~ModEngineView() {
    lv_obj_delete(_cnvModView);
    delete [] _drawBuffer;
}

void ModEngineView::updateLine(slr::frame_t nudge) {
    slr::frame_t sampleRate = slr::TimelineView::getTimelineView().sampleRate();
    slr::frame_t reminder = nudge % sampleRate;

    int x = static_cast<int>(sMath::lerp<slr::frame_t>(reminder, 0, sampleRate, 360, 1560));
    _points[0] = {x, 240};
    _points[1] = {x, 840};
    lv_line_set_points(_line, &_points[0], 2);
    lv_obj_move_to_index(_line, -1);
    lv_obj_invalidate(_line);
}

void ModEngineView::redrawCanvas(const std::string type, float rate, float offset) {
    fillCanvas(_cnvModView, 1200, 600, lv_color_hex(0x000000));
    if(type.compare("Sin") == 0) {
        for(int x=0; x<1200; ++x) {
            float sample = sMath::sin(2*M_PI * (x/1200.f) * rate + offset);
            int y = sMath::lerp(sample, 1.f, -1.f, 10.f, 590.f);
            lv_canvas_set_px(_cnvModView, x, y, RED_COLOR, LV_OPA_COVER);
        }

    } else if(type.compare("Saw") == 0) {
        int prev = 600/2;
        for(int x=0; x<1200; ++x) {
            float sample = sMath::atan(sMath::tan(2*M_PI*(x/1200.f)*rate+offset)) / 2;
            sample = sMath::clamp(sample, -1.f, 1.f);
            int y = sMath::lerp(sample, 1.f, -1.f, 10.f, 590.f);
            lv_canvas_set_px(_cnvModView, x, y, RED_COLOR, LV_OPA_COVER);
            
            if(sMath::abs(prev - y) > 10) {
                for(int py = 10; py<590; ++py) {
                    lv_canvas_set_px(_cnvModView, x, py, RED_COLOR, LV_OPA_COVER);
                }
            }
            prev = y;
        }
    } else if(type.compare("Square") == 0) {
        int prev = 0;
        for(int x=0; x<1200; ++x) {
            float sample = std::copysign(1.0f, std::sin(2 * M_PI * (x/1200.f) * rate + offset) );
            int y = sMath::lerp(sample, 1.f, -1.f, 10.f, 590.f);
            lv_canvas_set_px(_cnvModView, x, y, RED_COLOR, LV_OPA_COVER);
            if(prev != y) {
                for(int py = 10; py<590; ++py) {
                    lv_canvas_set_px(_cnvModView, x, py, RED_COLOR, LV_OPA_COVER);
                }
                prev = y;
            }
        }
    } else if(type.compare("Tri") == 0) {
        for(int x=0; x<1200; ++x) {
            float sample = sMath::asin(sMath::sin(2*M_PI*(x/1200.f)*rate+offset)) / 2;
            sample = sMath::clamp(sample, -1.f, 1.f);
            int y = sMath::lerp(sample, 1.f, -1.f, 10.f, 590.f);
            lv_canvas_set_px(_cnvModView, x, y, RED_COLOR, LV_OPA_COVER);
        }
    } else if(type.compare("Rand") == 0) {

    }
}


}
