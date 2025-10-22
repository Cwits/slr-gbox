// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/RouteManager.h"

#include "ui/display/primitives/UIContext.h"
#include "ui/display/layoutSizes.h"
#include "ui/display/defaultColors.h"
#include "logger.h"

#include "core/primitives/AudioRoute.h"
#include "core/Events.h"

#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"
#include "snapshots/DriverView.h"

#include <algorithm>

namespace UI {

RouteManager::RouteManager(BaseWidget * parent, UIContext * const uictx) :
    Popup(parent, uictx)
{
    _flags.isTap = true;
    setSize(LayoutDef::ROUTE_MANAGER_WIDTH, LayoutDef::ROUTE_MANAGER_HEIGHT);
    setPos(LayoutDef::ROUTE_MANAGER_X, LayoutDef::ROUTE_MANAGER_Y);
    setColor(lv_color_hex(0x858585));

    _text = new Label(this, "Routes for track ...");
    _text->setSize(800, 33);
    _text->setPos(400, 10);
    _text->setFont(&DEFAULT_FONT);
    _text->setTextColor(lv_color_hex(0x0f0fff));

    _applyBtn = new Button(this, "Apply");
    _applyBtn->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _applyBtn->setPos(LayoutDef::ROUTE_MANAGER_WIDTH - (LayoutDef::BUTTON_SIZE+20),
                        LayoutDef::ROUTE_MANAGER_HEIGHT - (LayoutDef::BUTTON_SIZE+20));
    _applyBtn->setCallback([this]() {
        LOG_INFO("Apply routes");
        this->_uictx->_popManager->disableRouteManager();
        // this->hide();
        // this->deactivate();
    });

    /* First item right side */
    _inputsText = new Label(this, "Receiving from:");
    _inputsText->setSize(250, 33);
    _inputsText->setPos(LayoutDef::ROUTE_CTL_LEFT_EXT_X, 50);
    _inputsText->setFont(&DEFAULT_FONT);
    _inputsText->setTextColor(BLACK_COLOR);

    /* left side */
    _outputsText = new Label(this, "Sending to:");
    _outputsText->setSize(250, 33);
    _outputsText->setPos(LayoutDef::ROUTE_CTL_RIGHT_EXT_X, 50);
    _outputsText->setFont(&DEFAULT_FONT);
    _outputsText->setTextColor(BLACK_COLOR);

    _inputs.reserve(5);
    _outputs.reserve(5);

    _inputs.clear();
    _outputs.clear();
    
    _nextInput._extint = nullptr;
    _nextInput._dropdown = nullptr;
    _nextInput._channelMap = nullptr;
    _nextInput._addRemoveButton = nullptr;
    
    _nextOutput._extint = nullptr;
    _nextOutput._dropdown = nullptr;
    _nextOutput._channelMap = nullptr;
    _nextOutput._addRemoveButton = nullptr;
}

RouteManager::~RouteManager() {
    delete _text;
    delete _inputsText;
    delete _outputsText;
    
    delete _applyBtn;

    for(auto &r : _inputs) {
        delete r._extint;
        delete r._dropdown;
        delete r._channelMap;
        delete r._addRemoveButton;
    }
    for(auto &r : _outputs) {
        delete r._extint;
        delete r._dropdown;
        delete r._channelMap;
        delete r._addRemoveButton;
    }
    
    delete _nextInput._extint;
    delete _nextInput._dropdown;
    delete _nextInput._channelMap;
    delete _nextInput._addRemoveButton;
    
    delete _nextOutput._extint;
    delete _nextOutput._dropdown;
    delete _nextOutput._channelMap;
    delete _nextOutput._addRemoveButton;
}

void RouteManager::update() {
    std::string text = "Routings for ";
    slr::AudioUnitView * view = slr::ProjectView::getProjectView().getUnitById(_currentUnitId);
    text.append(view->name());
    _text->setText(text);

    
    //routes where this id is targeting(sending from this id)
    //receiving from
    std::vector<slr::AudioRoute> tgts = slr::ProjectView::getProjectView().targetsForId(_currentUnitId);
    for(std::size_t i=0; i<tgts.size(); ++i) {
        Route r;
        slr::AudioRoute &in = tgts.at(i);
        std::string textDD;
        if(in._sourceType == slr::AudioRoute::Type::EXT) {
            textDD = "Driver";
        } else {
            slr::AudioUnitView * unit = slr::ProjectView::getProjectView().getUnitById(in._sourceId);
            textDD = unit->name();
        }

        int posy = (LayoutDef::ROUTE_LINE_HEIGHT+LayoutDef::ROUTE_LINE_MARGIN) * i;

        Button * extint = new Button(this, in._sourceType == slr::AudioRoute::Type::EXT ? "EXT" : "INT");
        extint->setSize(LayoutDef::ROUTE_CTL_LEFT_EXT_W, LayoutDef::ROUTE_CTL_LEFT_EXT_H);
        extint->setPos(LayoutDef::ROUTE_CTL_LEFT_EXT_X, LayoutDef::ROUTE_CTL_LEFT_EXT_Y+posy);
        extint->setFont(&DEFAULT_FONT);
        r._extint = extint;

        DropDown * drop = new DropDown(this);
        drop->button()->setSize(LayoutDef::ROUTE_CTL_LEFT_DD_W, LayoutDef::ROUTE_CTL_LEFT_DD_H);
        drop->button()->setPos(LayoutDef::ROUTE_CTL_LEFT_DD_X, LayoutDef::ROUTE_CTL_LEFT_DD_Y+posy);
        drop->setSelected(textDD);
        drop->setPos(LayoutDef::ROUTE_CTL_LEFT_DD_X, LayoutDef::ROUTE_CTL_LEFT_DD_Y+LayoutDef::ROUTE_LINE_HEIGHT+posy);
        r._dropdown = drop;

        Button * map = new Button(this, "Map");
        map->setSize(LayoutDef::ROUTE_CTL_LEFT_MAP_W, LayoutDef::ROUTE_CTL_LEFT_MAP_H);
        map->setPos(LayoutDef::ROUTE_CTL_LEFT_MAP_X, LayoutDef::ROUTE_CTL_LEFT_MAP_Y+posy);
        map->setFont(&DEFAULT_FONT);
        r._channelMap = map;

        Button * addremove = new Button(this, LV_SYMBOL_MINUS);
        addremove->setSize(LayoutDef::ROUTE_CTL_LEFT_ADD_W, LayoutDef::ROUTE_CTL_LEFT_ADD_H);
        addremove->setPos(LayoutDef::ROUTE_CTL_LEFT_ADD_X, LayoutDef::ROUTE_CTL_LEFT_ADD_Y+posy);
        addremove->setFont(&DEFAULT_FONT);
        addremove->setCallback([this]() {
            LOG_INFO("add Remove Route event");
        });
        r._addRemoveButton = addremove;

        _inputs.push_back(r);
    }

    //routes where this id is source(receiving from this id)
    //sending to
    std::vector<slr::AudioRoute> srcs = slr::ProjectView::getProjectView().sourcesForId(_currentUnitId);
    for(std::size_t i=0; i<srcs.size(); ++i) {
        Route r;
        slr::AudioRoute & out = srcs.at(i);
        std::string textDD;

        if(out._targetType == slr::AudioRoute::Type::EXT) {
            textDD = "Driver";
        } else {
            slr::AudioUnitView * au = slr::ProjectView::getProjectView().getUnitById(srcs.at(i)._targetId);// -> mixer
            textDD = au->name();
        }

        int posy = (LayoutDef::ROUTE_LINE_HEIGHT+LayoutDef::ROUTE_LINE_MARGIN) * i;

        Button * extint = new Button(this, out._targetType == slr::AudioRoute::Type::EXT ? "EXT" : "INT");
        extint->setSize(LayoutDef::ROUTE_CTL_RIGHT_EXT_W, LayoutDef::ROUTE_CTL_RIGHT_EXT_H);
        extint->setPos(LayoutDef::ROUTE_CTL_RIGHT_EXT_X, LayoutDef::ROUTE_CTL_RIGHT_EXT_Y+posy);
        extint->setFont(&DEFAULT_FONT);
        r._extint = extint;

        DropDown * drop = new DropDown(this);
        drop->button()->setSize(LayoutDef::ROUTE_CTL_RIGHT_DD_W, LayoutDef::ROUTE_CTL_RIGHT_DD_H);
        drop->button()->setPos(LayoutDef::ROUTE_CTL_RIGHT_DD_X, LayoutDef::ROUTE_CTL_RIGHT_DD_Y+posy);
        drop->setSelected(textDD);
        drop->setPos(LayoutDef::ROUTE_CTL_RIGHT_DD_X, LayoutDef::ROUTE_CTL_RIGHT_DD_Y+LayoutDef::ROUTE_LINE_HEIGHT+posy);
        r._dropdown = drop;

        Button * map = new Button(this, "Map");
        map->setSize(LayoutDef::ROUTE_CTL_RIGHT_MAP_W, LayoutDef::ROUTE_CTL_RIGHT_MAP_H);
        map->setPos(LayoutDef::ROUTE_CTL_RIGHT_MAP_X, LayoutDef::ROUTE_CTL_RIGHT_MAP_Y+posy);
        map->setFont(&DEFAULT_FONT);
        r._channelMap = map;

        Button * addremove = new Button(this, LV_SYMBOL_MINUS);
        addremove->setSize(LayoutDef::ROUTE_CTL_RIGHT_ADD_W, LayoutDef::ROUTE_CTL_RIGHT_ADD_H);
        addremove->setPos(LayoutDef::ROUTE_CTL_RIGHT_ADD_X, LayoutDef::ROUTE_CTL_RIGHT_ADD_Y+posy);
        addremove->setFont(&DEFAULT_FONT);
        r._addRemoveButton = addremove;

        _outputs.push_back(r);
    }

    // std::vector<slr::AudioUnitView> allunits = slr::ProjectView::getProjectView().allUnits();
    
    //new input controls
    std::size_t size = _inputs.size();
    int posy = (LayoutDef::ROUTE_LINE_HEIGHT*size)+(LayoutDef::ROUTE_LINE_MARGIN*size);

    slr::DriverView *dri = slr::DriverView::driverView();
    std::vector<std::string> items;
    items.push_back(std::string("Mono Left"));
    items.push_back(std::string("Mono Right"));
    items.push_back(std::string("Stereo"));
    
    Button * nexiint = new Button(this, "EXT");
    nexiint->setPos(LayoutDef::ROUTE_CTL_LEFT_EXT_X, LayoutDef::ROUTE_CTL_LEFT_EXT_Y+posy);
    nexiint->setSize(LayoutDef::ROUTE_CTL_LEFT_EXT_W, LayoutDef::ROUTE_CTL_LEFT_EXT_H);
    nexiint->setFont(&DEFAULT_FONT);
    nexiint->setCallback([this]() {
        std::string text = this->_nextInput._extint->text();
        if(text.compare("EXT") == 0) {
            this->_nextInput._extint->setText("INT");
            //fill drop
            std::vector<slr::AudioUnitView*> list = slr::ProjectView::getProjectView().unitList();
            std::vector<std::string> names;
            for(auto u : list) {
                names.push_back(u->name());
            }
            this->_nextInput._dropdown->setItems(names);
        } else {
            this->_nextInput._extint->setText("EXT");
            std::vector<std::string> items;
            items.push_back(std::string("Mono Left"));
            items.push_back(std::string("Mono Right"));
            items.push_back(std::string("Stereo"));
            this->_nextInput._dropdown->setItems(items);
        }
    });
    
    DropDown * ndd = new DropDown(this);
    ndd->button()->setPos(LayoutDef::ROUTE_CTL_LEFT_DD_X, LayoutDef::ROUTE_CTL_LEFT_DD_Y+posy);
    ndd->button()->setSize(LayoutDef::ROUTE_CTL_LEFT_DD_W, LayoutDef::ROUTE_CTL_LEFT_DD_H);
    ndd->setPos(LayoutDef::ROUTE_CTL_LEFT_DD_X, LayoutDef::ROUTE_CTL_LEFT_DD_Y+LayoutDef::ROUTE_LINE_HEIGHT+posy);
    ndd->setSize(LayoutDef::ROUTE_CTL_LEFT_DD_W, LayoutDef::ROUTE_CTL_LEFT_DD_H);
    ndd->setItems(items);

    Button * nmap = new Button(this, "MAP");
    nmap->setPos(LayoutDef::ROUTE_CTL_LEFT_MAP_X, LayoutDef::ROUTE_CTL_LEFT_MAP_Y+posy);
    nmap->setSize(LayoutDef::ROUTE_CTL_LEFT_MAP_W, LayoutDef::ROUTE_CTL_LEFT_MAP_H);
    nmap->setFont(&DEFAULT_FONT);

    Button * nadd = new Button(this, LV_SYMBOL_PLUS);
    nadd->setPos(LayoutDef::ROUTE_CTL_LEFT_ADD_X, LayoutDef::ROUTE_CTL_LEFT_ADD_Y+posy);
    nadd->setSize(LayoutDef::ROUTE_CTL_LEFT_ADD_W, LayoutDef::ROUTE_CTL_LEFT_ADD_H);
    nadd->setFont(&DEFAULT_FONT);
    nadd->setCallback([this]() {
        RouteManager::newRoute(this, true);
        //update here? or... where?
    });

    _nextInput._extint = nexiint;
    _nextInput._dropdown = ndd;
    _nextInput._channelMap = nmap;
    _nextInput._addRemoveButton = nadd;

    //new output
    size = _outputs.size();
    posy = (LayoutDef::ROUTE_LINE_HEIGHT*size)+(LayoutDef::ROUTE_LINE_MARGIN*size);

    Button * oexiint = new Button(this, "EXT");
    oexiint->setPos(LayoutDef::ROUTE_CTL_RIGHT_EXT_X, LayoutDef::ROUTE_CTL_RIGHT_EXT_Y+posy);
    oexiint->setSize(LayoutDef::ROUTE_CTL_RIGHT_EXT_W, LayoutDef::ROUTE_CTL_RIGHT_EXT_H);
    oexiint->setFont(&DEFAULT_FONT);
    oexiint->setCallback([this]() {
        std::string text = this->_nextOutput._extint->text();
        if(text.compare("EXT") == 0) {
            this->_nextOutput._extint->setText("INT");
            //fill drop
            std::vector<slr::AudioUnitView*> list = slr::ProjectView::getProjectView().unitList();
            std::vector<std::string> names;
            for(auto u : list) {
                names.push_back(u->name());
            }
            this->_nextOutput._dropdown->setItems(names);
        } else {
            this->_nextOutput._extint->setText("EXT");
            std::vector<std::string> items;
            items.push_back(std::string("Mono Left"));
            items.push_back(std::string("Mono Right"));
            items.push_back(std::string("Stereo"));
            this->_nextOutput._dropdown->setItems(items);
        }
    });

    DropDown * odd = new DropDown(this);
    odd->button()->setPos(LayoutDef::ROUTE_CTL_RIGHT_DD_X, LayoutDef::ROUTE_CTL_RIGHT_DD_Y+posy);
    odd->button()->setSize(LayoutDef::ROUTE_CTL_RIGHT_DD_W, LayoutDef::ROUTE_CTL_RIGHT_DD_H);
    odd->setPos(LayoutDef::ROUTE_CTL_RIGHT_DD_X, LayoutDef::ROUTE_CTL_RIGHT_DD_Y+LayoutDef::ROUTE_LINE_HEIGHT+posy);
    odd->setSize(LayoutDef::ROUTE_CTL_RIGHT_DD_W, LayoutDef::ROUTE_CTL_RIGHT_DD_H);
    odd->setItems(items);

    
    Button * omap = new Button(this, "MAP");
    omap->setPos(LayoutDef::ROUTE_CTL_RIGHT_MAP_X, LayoutDef::ROUTE_CTL_RIGHT_MAP_Y+posy);
    omap->setSize(LayoutDef::ROUTE_CTL_RIGHT_MAP_W, LayoutDef::ROUTE_CTL_RIGHT_MAP_H);
    omap->setFont(&DEFAULT_FONT);

    Button * oadd = new Button(this, LV_SYMBOL_PLUS);
    oadd->setPos(LayoutDef::ROUTE_CTL_RIGHT_ADD_X, LayoutDef::ROUTE_CTL_RIGHT_ADD_Y+posy);
    oadd->setSize(LayoutDef::ROUTE_CTL_RIGHT_ADD_W, LayoutDef::ROUTE_CTL_RIGHT_ADD_H);
    oadd->setFont(&DEFAULT_FONT);
    oadd->setCallback([this]() {
        RouteManager::newRoute(this, false);
        //update here? or... where?
    });

    _nextOutput._extint = oexiint;
    _nextOutput._dropdown = odd;
    _nextOutput._channelMap = omap;
    _nextOutput._addRemoveButton = oadd;

}

void RouteManager::forcedClose() {
    // LOG_INFO("Forced close route manager");
    for(Route &r : _outputs) {
        delete r._extint;
        delete r._dropdown;
        delete r._channelMap;
        delete r._addRemoveButton;
        r._extint = nullptr;
        r._dropdown = nullptr;
        r._channelMap = nullptr;
        r._addRemoveButton = nullptr;
    }

    _outputs.clear();
    
    for(Route &r : _inputs) {
        delete r._extint;
        delete r._dropdown;
        delete r._channelMap;
        delete r._addRemoveButton;
        r._extint = nullptr;
        r._dropdown = nullptr;
        r._channelMap = nullptr;
        r._addRemoveButton = nullptr;
    }

    _inputs.clear();

    delete _nextInput._extint;
    delete _nextInput._dropdown;
    delete _nextInput._channelMap;
    delete _nextInput._addRemoveButton;
    
    delete _nextOutput._extint;
    delete _nextOutput._dropdown;
    delete _nextOutput._channelMap;
    delete _nextOutput._addRemoveButton;
    
    _nextInput._extint = nullptr;
    _nextInput._dropdown = nullptr;
    _nextInput._channelMap = nullptr;
    _nextInput._addRemoveButton = nullptr;
    
    _nextOutput._extint = nullptr;
    _nextOutput._dropdown = nullptr;
    _nextOutput._channelMap = nullptr;
    _nextOutput._addRemoveButton = nullptr;
}

void RouteManager::liveUpdate() {
    forcedClose();
    update();
    // lv_obj_invalidate(lvhost());
}

bool RouteManager::handleTap(GestLib::TapGesture & tap) {
    LOG_INFO("pos x %d y %d", tap.x, tap.y);

    return true;
}

void RouteManager::newRoute(RouteManager * rm, bool isInput) {
    slr::AudioRoute newroute;
    for(int i=0; i<32; ++i) {
        newroute._channelMap[i] = -1;
    }
    if(isInput) {            
        const std::string io = rm->_nextInput._extint->text();
        if(io.compare("EXT") == 0) {
            newroute._sourceType = slr::AudioRoute::Type::EXT;
            newroute._sourceId = 0;
                
            const std::string id = rm->_nextInput._dropdown->selectedItem();
            // newroute._sourceId = std::stoi(id);
            //TODO: issue here... Stereo thing is working correctly, 
            //but when it is mono than channels was swapped
            //(mono left = 0 both gave right and vice versa)
            if(id.compare("Mono Left") == 0) {
                newroute._channelMap[0] = 1; //that is weird...
                newroute._channelMap[1] = 1;
            } else if(id.compare("Mono Right") == 0) {
                newroute._channelMap[0] = 0;
                newroute._channelMap[1] = 0;
            } else if(id.compare("Stereo") == 0) {
                newroute._channelMap[0] = 0;
                newroute._channelMap[1] = 1;
            }
        } else {
            newroute._sourceType = slr::AudioRoute::Type::INT;
            const std::string name = rm->_nextInput._dropdown->selectedItem();
            std::vector<slr::AudioUnitView*> list = slr::ProjectView::getProjectView().unitList();
            bool found = false;
            slr::ID id = 0;
            for(auto unit : list) {
                if(name.compare(unit->name()) == 0) {
                    found = true;
                    id = unit->id();
                    break;
                }
            }

            if(!found) {
                LOG_WARN("Failed to find unit named %s in unit list", name.c_str());
                return;
            }
                
            newroute._sourceId = id;
            newroute._channelMap[0] = 0;
            newroute._channelMap[1] = 1;
        }
            
        newroute._targetType = slr::AudioRoute::Type::INT;
        newroute._targetId = rm->_currentUnitId;
    } else {
        //output thing
        const std::string io = rm->_nextOutput._extint->text();
        if(io.compare("EXT") == 0) {
            newroute._targetType = slr::AudioRoute::Type::EXT;
            newroute._targetId = 0;

            const std::string id = rm->_nextOutput._dropdown->selectedItem();
            if(id.compare("Mono Left") == 0) {
                newroute._channelMap[0] = 1;
                newroute._channelMap[1] = 1;
            } else if(id.compare("Mono Right") == 0) {
                newroute._channelMap[0] = 0;
                newroute._channelMap[1] = 0;
            } else if(id.compare("Stereo") == 0) {
                newroute._channelMap[0] = 0;
                newroute._channelMap[1] = 1;
            }
        } else {
            newroute._targetType = slr::AudioRoute::Type::INT;
            const std::string name = rm->_nextOutput._dropdown->selectedItem();
            std::vector<slr::AudioUnitView*> list = slr::ProjectView::getProjectView().unitList();
            bool found = false;
            slr::ID id = 0;
            for(auto unit : list) {
                if(name.compare(unit->name()) == 0) {
                    found = true;
                    id = unit->id();
                    break;
                }
            }

            if(!found) {
                LOG_WARN("Failed to find unit named %s in unit list", name.c_str());
                return;
            }

            newroute._targetId = id;
            newroute._channelMap[0] = 0;
            newroute._channelMap[1] = 1;
        }

            newroute._sourceId = rm->_currentUnitId;
            newroute._sourceType = slr::AudioRoute::Type::INT;
    }

    slr::Events::AddNewRoute r;
    r.route = newroute;
    slr::EmitEvent(r);
}

}