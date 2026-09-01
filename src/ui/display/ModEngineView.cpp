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
#include "snapshots/ModulationView.h"
#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"
#include "snapshots/ParameterArrayView.h"
#include "snapshots/ParameterView.h"
#include "core/Actions.h"
#include "ui/uiutility.h"

#include "common/ModEngineCommon.h"
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

ModulationUI::ModulationUI(const std::shared_ptr<slr::ModulationPatternView> view) :
    _view(view)
{

}

slr::ID ModulationUI::id() const { 
    return _view->id();
}


ModEngineView::ModEngineView(BaseWidget * parent, UIContext * const uictx) :
    View(parent, uictx)
{
    setPos(LayoutDef::STEP_SEQ_X, LayoutDef::STEP_SEQ_Y);
    setSize(LayoutDef::STEP_SEQ_WIDTH, LayoutDef::STEP_SEQ_HEIGHT);
    setColor(lv_color_hex(0x06e17e));

    LABEL(_lblModNumText, LayoutDef::DEFAULT_MARGIN, LayoutDef::DEFAULT_MARGIN+10, 200, 40, "Modulation: ");
    LABEL(_lblModNum,     210,                       LayoutDef::DEFAULT_MARGIN+10, 100, 40, "x / x");
    
    BUTTON(_btnNextMod, 1600, LayoutDef::DEFAULT_MARGIN, LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE, LV_SYMBOL_RIGHT);
    _btnNextMod->setCallback([this]() { 
        if(this->_currentSelectedMod+1 < _modUIs.size())
            this->showByPos(this->_currentSelectedMod+1);
    });
    BUTTON(_btnPrevMod, 1500, LayoutDef::DEFAULT_MARGIN, LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE, LV_SYMBOL_LEFT);
    _btnPrevMod->setCallback([this]() { 
        if(this->_currentSelectedMod > 0)
            this->showByPos(this->_currentSelectedMod-1);
    });
    BUTTON(_btnNewMod,  1700, LayoutDef::DEFAULT_MARGIN, LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE, LV_SYMBOL_PLUS);
    _btnNewMod->setCallback([this]() {
        auto act = std::make_unique<slr::Actions::CreateNewModulation>();
        slr::EmitAction(std::move(act));
    });

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
        ModulationUI * ui = this->currentMod();
        if(!ui) return;

        auto act = std::make_unique<slr::Actions::ModifyModulation>();
        act->modulationId = ui->id();
        if(selected.compare("Sin") == 0) act->shape = slr::ModulationShape::sin;
        else if(selected.compare("Saw") == 0) act->shape = slr::ModulationShape::saw;
        else if(selected.compare("Square") == 0) act->shape = slr::ModulationShape::square;
        else if(selected.compare("Tri") == 0) act->shape = slr::ModulationShape::tri;
        slr::EmitAction(std::move(act));
    });

    LABEL(_lblRateText, 300+50, y, 100, 40, "Rate: ");
    LABEL(_lblRate,     380+50, y, 100, 40, "x Hz");
    _lblRate->setTapCallback([this]() {
        std::string initText = this->_lblRate->text().substr(0, this->_lblRate->text().size()-2);
        this->_uictx->_popManager->enableKeyboard(
            initText,
            [this](const std::string & text) {
                ModulationUI * ui = this->currentMod();
                if(!ui) return;
                
                float rate = std::stof(text);

                auto act = std::make_unique<slr::Actions::ModifyModulation>();
                act->modulationId = ui->id();
                act->rate = rate;
                slr::EmitAction(std::move(act));
            }
        );
    });

    BUTTON(_btnRate,    300+50, y+40, LayoutDef::BUTTON_SIZE+80, 40, "Hz/Beat");
    
    LABEL(_lblPhaseText, 600+50, y, 100, 40, "Phase: ");
    LABEL(_lblPhase,   710+50, y, 100, 40, "x");
    _lblPhase->setTapCallback([this]() {
        this->_uictx->_popManager->enableKeyboard(
            this->_lblPhase->text(),
            [this](const std::string & text) {
                ModulationUI * ui = this->currentMod();
                if(!ui) return;
                
                float phase = std::stof(text);
                                
                auto act = std::make_unique<slr::Actions::ModifyModulation>();
                act->modulationId = ui->id();
                act->phase = phase;
                slr::EmitAction(std::move(act));
            }
        );
    });

    // LABEL(_lblRandSeedText, 900+50, y, 100, 40, "Seed: ");
    // LABEL(_lblRandSeed,    1010+50, y, 100, 40, "0");
    // LABEL(_lblLengthText, 1200+50, y, 120, 40, "Length: ");
    // LABEL(_lblLength,   1200+140+50, y, 100, 40, "Err");

    // BUTTON(_btnLength,  1200+50, y+40, LayoutDef::BUTTON_SIZE+80, 40, "Sec/Beat");

    BUTTON(_btnTargetManager, 1700, y, LayoutDef::BUTTON_SIZE*2, LayoutDef::BUTTON_SIZE, "Targets");
    _btnTargetManager->setCallback([this]() {
        this->_tpop->clear();
        this->_tpop->update();
        this->_uictx->_popManager->enableModEngineTargetManager();
    });

    LABEL(_lblMinimalText, 20, 500, 300, 40, "Minimal value:");
    LABEL(_lblMinimal, 150, 560, 300, 40, "x");
    _lblMinimal->setTapCallback([this]() {
        this->_uictx->_popManager->enableKeyboard(
            this->_lblMinimal->text(),
            [this](const std::string & text) {
                ModulationUI * ui = this->currentMod();
                if(!ui) return;

                float min = std::stof(text);
                
                auto act = std::make_unique<slr::Actions::ModifyModulation>();
                act->modulationId = ui->id();
                act->min = min;
                slr::EmitAction(std::move(act));

            }
        );
    });

    LABEL(_lblMaximalText, 20, 640, 300, 40, "Maximal value:");
    LABEL(_lblMaximal, 150, 700, 300, 40, "x");
    _lblMaximal->setTapCallback([this]() {
        this->_uictx->_popManager->enableKeyboard(
            this->_lblMaximal->text(),
            [this](const std::string & text) {
                ModulationUI * ui = this->currentMod();
                if(!ui) return;

                float max = std::stof(text);
                
                auto act = std::make_unique<slr::Actions::ModifyModulation>();
                act->modulationId = ui->id();
                act->max = max;
                slr::EmitAction(std::move(act));
            }
        );
    });


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
    
    _currentSelectedMod = 0;
}

ModEngineView::~ModEngineView() {
    lv_obj_delete(_cnvModView);
    delete [] _drawBuffer;
}

ModulationUI * ModEngineView::currentMod() const { 
    if(_modUIs.size() == 0) return nullptr; 
    if(_currentSelectedMod > _modUIs.size()) return nullptr;
    return _modUIs.at(_currentSelectedMod).get();
}

void ModEngineView::createModUI(const std::shared_ptr<slr::ModulationPatternView> view) {
    _modUIs.push_back(std::make_unique<ModulationUI>(view));
    
    showByPos(_modUIs.size()-1);
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

void ModEngineView::showByPos(std::size_t pos) {
    // assert(pos != 0);
    assert(pos < _modUIs.size());
    _currentSelectedMod = pos;

    ModulationUI * ui = currentMod();
    if(!ui) return;

    std::string text; //ID should be here
    text.append(std::to_string(pos+1)); text.append(" / "); text.append(std::to_string(_modUIs.size()));
    _lblModNum->setText(text);
    text.clear();

    switch(ui->view()->shape()) {
        case(slr::ModulationShape::sin): text.append("Sin"); break;
        case(slr::ModulationShape::saw): text.append("Saw"); break;
        case(slr::ModulationShape::square): text.append("Square"); break;
        case(slr::ModulationShape::tri): text.append("Tri"); break;
    }
    _ddType->setSelected(text);
    text.clear();

    text.append(std::to_string(ui->view()->rate()));
    text.append(" Hz");
    _lblRate->setText(text);
    _lblPhase->setText(std::to_string(ui->view()->phase()));
    _lblMinimal->setText(std::to_string(ui->view()->min()));
    _lblMaximal->setText(std::to_string(ui->view()->max()));

    redrawCanvas(_ddType->selectedItem(), ui->view()->rate(), ui->view()->phase());
}

void ModEngineView::redrawCanvas(const std::string type, float rate, float offset) {
    fillCanvas(_cnvModView, 1200, 600, lv_color_hex(0x000000));
    if(type.compare("Sin") == 0) {
        for(int x=0; x<1200; ++x) {
            // float sample = sMath::sin(2*M_PI * (x/1200.f) * rate + offset);
            float sample = sMath::sineWave(1.0f, static_cast<float>(x), 1200.f, rate, offset);
            int y = sMath::lerp(sample, 1.f, -1.f, 10.f, 590.f);
            lv_canvas_set_px(_cnvModView, x, y, RED_COLOR, LV_OPA_COVER);
        }

    } else if(type.compare("Saw") == 0) {
        int prev = 600/2;
        for(int x=0; x<1200; ++x) {
            float sample = sMath::sawtoothWave(0.5f, static_cast<float>(x), 1200.f, rate, offset);
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
            float sample = sMath::squareWave(1.0f, static_cast<float>(x), 1200.f, rate, offset);
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
            float sample = sMath::triangleWave(1.0f, static_cast<float>(x), 1200.0f, rate, offset);
            int y = sMath::lerp(sample, 1.f, -1.f, 10.f, 590.f);
            lv_canvas_set_px(_cnvModView, x, y, RED_COLOR, LV_OPA_COVER);
        }
    } else if(type.compare("Rand") == 0) {
        //TODO: TBD
    }
}

void ModEngineView::pollUIUpdate() {
    ModulationUI * ui = currentMod();
    if(!ui) return;

    uint64_t ver = ui->view()->version();
    if(ui->_uiVersion == ver) return;
    ui->_uiVersion = ver;

    switch(ui->view()->shape()) {
        case(slr::ModulationShape::sin): _ddType->setSelected("Sin"); break;
        case(slr::ModulationShape::saw): _ddType->setSelected("Saw"); break;
        case(slr::ModulationShape::square): _ddType->setSelected("Square"); break;
        case(slr::ModulationShape::tri): _ddType->setSelected("Tri"); break;
    }

    _lblRate->setText(std::to_string(ui->view()->rate()));
    _lblPhase->setText(std::to_string(ui->view()->phase()));
    _lblMinimal->setText(std::to_string(ui->view()->min()));
    _lblMaximal->setText(std::to_string(ui->view()->max()));

    redrawCanvas(_ddType->selectedItem(), ui->view()->rate(), ui->view()->phase());
}

ModEngineTargetManager::ModEngineTargetManager(BaseWidget *parent, ModEngineView *sParent, UIContext * const uictx) :
    Popup(parent, uictx),
    _view(sParent)
{
    setSize(LayoutDef::ROUTE_MANAGER_WIDTH, LayoutDef::ROUTE_MANAGER_HEIGHT);
    setPos(LayoutDef::ROUTE_MANAGER_X, LayoutDef::ROUTE_MANAGER_Y);
    setColor(lv_color_hex(0xa415f7));
    
    LABEL(_lblCurrentMod, LayoutDef::ROUTE_MANAGER_WIDTH/2 - 150, 40, 300, 40, "Modulation: x/x");

    BUTTON(_btnNextMod, LayoutDef::ROUTE_MANAGER_WIDTH - (LayoutDef::BUTTON_SIZE + LayoutDef::DEFAULT_MARGIN), LayoutDef::DEFAULT_MARGIN, LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE, LV_SYMBOL_NEXT);
    BUTTON(_btnPrevMod, LayoutDef::ROUTE_MANAGER_WIDTH - (LayoutDef::BUTTON_SIZE*2 + LayoutDef::DEFAULT_MARGIN*2), LayoutDef::DEFAULT_MARGIN, LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE, LV_SYMBOL_PREV);
    
    _ddTargetSelector = std::make_unique<DropDown>(this);
    _ddParameterSelector = std::make_unique<DropDown>(this);
    _lblAmmount = std::make_unique<Label>(this);
    _btnAddTarget = std::make_unique<Button>(this, LV_SYMBOL_PLUS);
    
    _ddTargetSelector->setSize(300, 40);
    _ddParameterSelector->setSize(300, 40);
    _lblAmmount->setSize(200, 40);
    _btnAddTarget->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);

    _ddTargetSelector->close();
    _ddTargetSelector->hide();
    _ddParameterSelector->close();
    _ddParameterSelector->hide();
    _lblAmmount->hide();
    _btnAddTarget->hide();
    
    hide();
}

ModEngineTargetManager::~ModEngineTargetManager() {

}

void ModEngineTargetManager::update() {
    ModulationUI * ui = _view->currentMod();
    if(!ui) {
        deactivate();
        hide();
    }

    std::string num = std::to_string(ui->id());
    std::string text = "Modulation: ";
    text.append(num); text.append(" / "); text.append(std::to_string(_view->_modUIs.size()));
    _lblCurrentMod->setText(text);

    _targets.clear();
    slr::ModTargetArray targets = ui->view()->targets();
    slr::ProjectView &prv = slr::ProjectView::getProjectView();

    int x = 20;
    int y = 100;

    for(int i=0; i<targets.maxTargets(); ++i) {
        slr::ModulationTarget & mt = targets._raw[i];
        if(mt.type == slr::ModulationTargetType::ERROR) continue;

        Targets t;
        std::string text;
        switch(mt.type) {
            case(slr::ModulationTargetType::modulation): { text.append("Modulation "); text.append(std::to_string(mt.targetID)); } break;
            case(slr::ModulationTargetType::stepSequence): { text.append("Sequence "); text.append(std::to_string(mt.targetID)); } break;
            case(slr::ModulationTargetType::unit): { text.append(prv.getUnitById(mt.targetID)->name()); } break;
        }
        LABEL(t._lblName, x, y+20, 300, 40, text);
            
        text.clear();
        switch(mt.type) {
            case(slr::ModulationTargetType::modulation): { text.append(std::to_string(mt.parameterID)); } break;
            case(slr::ModulationTargetType::stepSequence): { text.append(std::to_string(mt.parameterID)); } break;
            case(slr::ModulationTargetType::unit): { text.append(std::to_string(mt.parameterID)); } break;
        } //must contain name of parameter
        LABEL(t._lblParameter, x+350, y+20, 300, 40, text);
        LABEL(t._lblAmmount, x+700, y+20, 200, 40, "100%");
        BUTTON(t._btnDeleteTarget, x+920, y, LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE, LV_SYMBOL_TRASH);
        t._btnDeleteTarget->setCallback([name = t._lblName.get(), par = t._lblParameter.get(), this]() {
            slr::ID targetID = 0;
            slr::ID parID = 0;
            slr::ModulationTargetType type = slr::ModulationTargetType::ERROR;

            std::string tmpname = name->text();
            std::string tmp = tmpname.substr(0, tmpname.find_first_of(' '));

            if(tmp.compare("Modulation") == 0) {
                type = slr::ModulationTargetType::modulation;
            } else if(tmp.compare("Sequence") == 0) {
                type = slr::ModulationTargetType::stepSequence;
            } else {
                type = slr::ModulationTargetType::unit;
                //unit
                slr::AudioUnitView *v = slr::ProjectView::getProjectView().findUnitByName(tmpname);
                if(!v) { LOG_WARN("Unit %s not found"); return; }

                targetID = v->id();

                slr::ParameterArrayView &pav = v->allParameters();
                std::string parName = par->text();
                for(std::size_t i=0; i<pav.count(); ++i) {
                    if(pav[i]->name().compare(parName) == 0) {
                        parID = i;
                        break;
                    }
                }
            }

            if(type == slr::ModulationTargetType::ERROR || targetID == 0) {
                LOG_ERROR("Something went wrong");
                return;
            }

            auto act = std::make_unique<slr::Actions::ModifyModulationTarget>();
            act->modulationID = this->_view->currentMod()->id();
            act->addOrRemove = false;
            act->type = type;
            act->targetID = targetID;
            act->parameterID = parID;
            slr::EmitAction(std::move(act));

        }); //end of Delete Target Button Callback

        _targets.push_back(std::move(t));
        y += (LayoutDef::BUTTON_SIZE + LayoutDef::DEFAULT_MARGIN);
    }
    
    //prepare dropdown line
    std::vector<std::string> tgtexts;
    tgtexts.push_back("Not Selected");
    for(const slr::AudioUnitView * auv : prv.unitList()) {
        tgtexts.push_back(auv->name());
    }
    _ddTargetSelector->setItems(tgtexts);
    _ddTargetSelector->selectedCallback([this](std::string res) {
        std::string find = res.substr(0, res.find_first_of(' '));
        if(find.compare("Modulation") == 0) {

        } else if(find.compare("Sequence") == 0) {

        } else {
            //unit
            slr::AudioUnitView *v = slr::ProjectView::getProjectView().findUnitByName(res);
            if(!v) { LOG_WARN("Unit %s not found"); return; }

            slr::ParameterArrayView &pav = v->allParameters();
            
            std::vector<std::string> names;
            for(std::size_t i=0; i<pav.count(); ++i) {
                names.push_back(pav[i]->name());
            }

            this->_ddParameterSelector->setItems(names);
        }
    });

    _ddParameterSelector->setSelected("Select target");
    _btnAddTarget->setCallback([this]() {
        slr::ModulationTargetType type = slr::ModulationTargetType::ERROR;
        slr::ID targetID = 0;
        slr::ID parID = 0;
        std::string tmp = this->_ddTargetSelector->selectedItem();
        std::string find = tmp.substr(0, tmp.find_first_of(' '));

        //find targetID
        if(find.compare("Modulation") == 0) {
            type = slr::ModulationTargetType::modulation;
        } else if(find.compare("Sequence") == 0) {
            type = slr::ModulationTargetType::stepSequence;
        } else {
            //unit
            type = slr::ModulationTargetType::unit;

            slr::AudioUnitView *v = slr::ProjectView::getProjectView().findUnitByName(tmp);
            if(!v) { LOG_WARN("Unit %s not found"); return; }

            targetID = v->id();

            slr::ParameterArrayView &pav = v->allParameters();
            std::string parName = this->_ddParameterSelector->selectedItem();
            for(std::size_t i=0; i<pav.count(); ++i) {
                if(pav[i]->name().compare(parName) == 0) {
                    parID = i;
                    break;
                }
            }
        }

        if(type == slr::ModulationTargetType::ERROR || targetID == 0) {
            LOG_ERROR("Something went wrong");
            return;
        }

        auto act = std::make_unique<slr::Actions::ModifyModulationTarget>();
        act->modulationID = this->_view->currentMod()->id();
        act->addOrRemove = true;
        act->type = type;
        act->targetID = targetID;
        act->parameterID = parID;
        slr::EmitAction(std::move(act));
    });
    
    _ddTargetSelector->setPos(x, y+20);
    _ddParameterSelector->setPos(x+350, y+20);
    _lblAmmount->setPos(x+700, y+20);
    _lblAmmount->setText("100%");
    _btnAddTarget->setPos(x+920, y);

    _ddTargetSelector->show(); _ddTargetSelector->close();
    _ddParameterSelector->show(); _ddParameterSelector->close();
    _lblAmmount->show();
    _btnAddTarget->show();
}


void ModEngineTargetManager::clear() {
    
}


}
