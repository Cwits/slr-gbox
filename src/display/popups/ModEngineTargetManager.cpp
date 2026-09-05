// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display/popups/ModEngineTargetManager.h"

#include "display/popups/PopupManager.h"

#include "display/primitives/Label.h"
#include "display/primitives/Button.h"
#include "display/primitives/DropDown.h"

#include "display/utility/UIContext.h"
#include "display/utility/layoutSizes.h"
#include "display/utility/defaultColors.h"
#include "display/utility/defaultStyles.h"
#include "display/elements/ModEngineView.h"

#include "display/utility/Macros.h"


#include "snapshots/TimelineView.h"
#include "snapshots/ModulationView.h"
#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"
#include "snapshots/ParameterArrayView.h"
#include "snapshots/ParameterView.h"
#include  "core/actions/Actions.h"
#include "common/uiutility.h"

#include "common/ModEngineCommon.h"
#include "common/Math.h"
#include "common/logger.h"

#include <cmath>

namespace UI {


ModEngineTargetManager::ModEngineTargetManager(BaseWidget *parent, ModEngineView *sParent, UIContext * const uictx) :
    Popup(parent, uictx),
    _view(sParent)
{
    setSize(Layout::ROUTE_MANAGER_WIDTH, Layout::ROUTE_MANAGER_HEIGHT);
    setPos(Layout::ROUTE_MANAGER_X, Layout::ROUTE_MANAGER_Y);
    setColor(lv_color_hex(0xa415f7));
    
    LABEL(_lblCurrentMod, Layout::ROUTE_MANAGER_WIDTH/2 - 150, 40, 300, 40, "Modulation: x/x");

    BUTTON(_btnNextMod, Layout::ROUTE_MANAGER_WIDTH - (Layout::Button + Layout::Margin), Layout::Margin, Layout::Button, Layout::Button, LV_SYMBOL_NEXT);
    BUTTON(_btnPrevMod, Layout::ROUTE_MANAGER_WIDTH - (Layout::Button*2 + Layout::Margin*2), Layout::Margin, Layout::Button, Layout::Button, LV_SYMBOL_PREV);
    
    _ddTargetSelector = std::make_unique<DropDown>(this);
    _ddParameterSelector = std::make_unique<DropDown>(this);
    _lblAmmount = std::make_unique<Label>(this);
    _btnAddTarget = std::make_unique<Button>(this, LV_SYMBOL_PLUS);
    
    _ddTargetSelector->setSize(300, 40);
    _ddParameterSelector->setSize(300, 40);
    _lblAmmount->setSize(200, 40);
    _btnAddTarget->setSize(Layout::Button, Layout::Button);

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
        BUTTON(t._btnDeleteTarget, x+920, y, Layout::Button, Layout::Button, LV_SYMBOL_TRASH);
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
        y += (Layout::Button + Layout::Margin);
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


}
