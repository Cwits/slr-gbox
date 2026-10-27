// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display/popups/RouteManager.h"

#include "display/primitives/Button.h"
#include "display/primitives/Checkbox.h"
#include "display/primitives/Label.h"
#include "display/primitives/DropDown.h"
#include "display/utility/UIContext.h"
#include "display/utility/layoutSizes.h"
#include "display/utility/DefaultColors.h"
#include "display/utility/DefaultStyles.h"
#include "display/utility/Macros.h"
#include "common/logger.h"

#include "core/primitives/AudioRoute.h"
#include "core/primitives/MidiRoute.h"
#include "core/primitives/MidiPort.h"
#include "core/MidiController.h"
#include "core/ControlEngine.h"
#include "core/actions/Actions.h"

#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"
#include "snapshots/DriverView.h"

#include "common/uiControls.h"

#include <algorithm>

std::string NONE_STRING = "None";

namespace UI {

RouteManager::RouteManager(BaseWidget * parent, UIContext * const uictx) :
    Popup(parent, uictx)
{
    _currentUnitId = 0;
    setSize(Layout::ROUTE_MANAGER_WIDTH, Layout::ROUTE_MANAGER_HEIGHT);
    setPos(Layout::ROUTE_MANAGER_X, Layout::ROUTE_MANAGER_Y);
    addStyle(&Style::PopupDefault);

    LABEL(_lblTargetNameStatic, 450, 60, 300, 40, "Routes for");
    
    _ddCurrentUnit = std::make_unique<DropDown>(this);
    _ddCurrentUnit->setSize(600, Layout::Button);
    _ddCurrentUnit->setPos(650, 40);
    std::string tmptext = "None";
    _ddCurrentUnit->setSelected(tmptext);
    _ddCurrentUnit->selectedCallback([this, drop = _ddCurrentUnit.get()](const std::string selected){
        std::vector<slr::AudioUnitView*> list = slr::ProjectView::getProjectView().unitList();
        auto it = std::find_if(
            list.begin(),
            list.end(),
            [&selected](const slr::AudioUnitView *v) {
                return v->name().compare(selected) == 0;
            }
        );
        if(it == list.end()) {
            //failed to find
            return;
        }

        this->_currentUnitId = (*it)->id();
        // std::string sel = selected;
        // drop->setSelected(sel);
        this->liveUpdate();
    });

    _ddCurrentUnit->button()->touchDownCallback([this, drop = _ddCurrentUnit.get()](const GestLib::TouchDownEvent &td) -> bool {
        drop->button()->setColor(Colors::ButtonPressed);
        std::vector<slr::AudioUnitView*> list = slr::ProjectView::getProjectView().unitList();
        std::vector<std::string> names;
        for(const slr::AudioUnitView *av : list) {
            names.push_back(av->name());
        }

        drop->setItems(names);
        return true;
    });

    /* First item right side */
    LABEL(_lblInputTextStatic, Layout::ROUTE_CTL_LEFT_EXT_X, 150-35, 250, 33, "Receiving From");

    /* left side */
    LABEL(_lblOutputTextStatic, Layout::ROUTE_CTL_RIGHT_EXT_X, 150-35, 250, 33, "Sending To");

    BUTTON(_btnAudioTab, 10, 40, 200, Layout::Button, "Audio Routes");
    _btnAudioTab->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        this->_midiTab->hide();
        this->_audioTab->show();
        return true;
    });

    BUTTON(_btnMidiTab, 200+Layout::Margin, 40, 200, Layout::Button, "Midi Routes");
    _btnMidiTab->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
        this->_audioTab->hide();
        this->_midiTab->show();
        return true;
    });

    _audioTab = std::make_unique<AudioTab>(this, _currentUnitId, this);
    _midiTab = std::make_unique<MidiTab>(this, _currentUnitId, this);

    _chbMidiThru = std::make_unique<Checkbox>(this);
    _chbMidiThru->setPos(1300, 40);
    _chbMidiThru->checkCallback([this](bool isChecked) {
        // slr::AudioUnitView * view = slr::ProjectView::getProjectView().getUnitById(_currentUnitId);

        // slr::Events::ToggleMidiThru e = {
        //     .targetId = view->id(),
        //     .newState = view->isMidiThru() ? false : true
        // };
        //     action->finished = [ptr = _rm]() {
        //     UIControls::DisplayTask([ptr]() {
        //         ptr->liveUpdate();
        //     });
        // };
        // slr::EmitEvent(e);
    });

    LABEL(_lblMidiThruStatic, 1300+10+Layout::CHECKBOX, 55, 200, 40, "Midi Thru");
    
    _chbOmniHwInput = std::make_unique<Checkbox>(this);
    _chbOmniHwInput->setPos(1500, 40);
    _chbOmniHwInput->checkCallback([this](bool isChecked) {
        // slr::AudioUnitView * view = slr::ProjectView::getProjectView().getUnitById(this->_currentUnitId);
        // if(!view) {
        //     LOG_ERROR("Failed to find unit view for id %u", this->_currentUnitId);
        //     return;
        // }

        // slr::Events::ToggleOmniHwInput e = {
        //     .targetId = view->id(),
        //     .newState = view->isOmniHwInput() ? false : true
        // };action->finished = [ptr = _rm]() {
        //     UIControls::DisplayTask([ptr]() {
        //         ptr->liveUpdate();
        //     });
        // };
        // slr::EmitEvent(e);
    });

    LABEL(_lblOmniHwInputStatic, 1500+10+Layout::CHECKBOX, 55, 300, 40, "Omni HW");
}

RouteManager::~RouteManager() { 
}

void RouteManager::update() {
    // if(!active()) return;
    if(_currentUnitId == 0) {
        LOG_WARN("Id shouldn't be zero");
        return;
    }
    
    slr::AudioUnitView * view = slr::ProjectView::getProjectView().getUnitById(_currentUnitId);
    std::string name = view->name();
    _ddCurrentUnit->setSelected(name);
    
    _audioTab->update();
    _midiTab->update();

    _chbMidiThru->setValue(view->isMidiThru());
    _chbOmniHwInput->setValue(view->isOmniHwInput());
}

void RouteManager::forcedClose() {
    // LOG_INFO("Forced close route manager");
    _audioTab->forcedClose();
    _midiTab->forcedClose();
}

void RouteManager::liveUpdate() {
    forcedClose();
    update();
}

RouteManager::AudioTab::AudioTab(BaseWidget *parent, const slr::ID &id, RouteManager *rm)
    : BaseWidget(parent, true),
    _currentId(id),
    _rm(rm)
{
    setPos(0, 150);
    setSize(Layout::ROUTE_MANAGER_WIDTH, Layout::ROUTE_MANAGER_HEIGHT-150);
    // setColor(lv_color_hex(0xac0000));
    addStyle(&Style::PopupDefault);

    _inputs.reserve(5);
    _outputs.reserve(5);

    _inputs.clear();
    _outputs.clear();
    
    show();
}

RouteManager::AudioTab::~AudioTab() {
    _inputs.clear();
    _outputs.clear();
}

void RouteManager::AudioTab::update() {
    const std::vector<slr::AudioRoute> routes = slr::ProjectView::getProjectView().audioRoutes();
    
    for(const slr::AudioRoute &r : routes) {
        if(r._sourceId != _currentId && r._targetId != _currentId) continue;

        if(r._targetId == _currentId && r._targetType == slr::AudioRoute::Type::INT) {
            int posy = Layout::ROUTE_LINE_MARGIN + 
                        (Layout::ROUTE_LINE_HEIGHT*_inputs.size()) +
                        (Layout::ROUTE_LINE_MARGIN*_inputs.size());
            addAsInput(false, r, posy);
        } else if(r._sourceId == _currentId && r._sourceType == slr::AudioRoute::Type::INT) {
            int posy = Layout::ROUTE_LINE_MARGIN + 
                        (Layout::ROUTE_LINE_HEIGHT*_outputs.size()) +
                        (Layout::ROUTE_LINE_MARGIN*_outputs.size());
            
            addAsOutput(false, r, posy);
        }
    }   //(gdb) p parent->spec_attr->child_cnt

    int posy = Layout::ROUTE_LINE_MARGIN + 
                (Layout::ROUTE_LINE_HEIGHT*_inputs.size()) +
                (Layout::ROUTE_LINE_MARGIN*_inputs.size());
    slr::AudioRoute dummy;
    addAsInput(true, dummy, posy);

    posy = Layout::ROUTE_LINE_MARGIN + 
            (Layout::ROUTE_LINE_HEIGHT*_outputs.size()) +
            (Layout::ROUTE_LINE_MARGIN*_outputs.size());
    addAsOutput(true, dummy, posy);
}

void RouteManager::AudioTab::forcedClose() {
    _outputs.clear();
    _inputs.clear();

    _nextInput.reset();
    _nextOutput.reset();
}

void RouteManager::AudioTab::addAsInput(bool isNew, const slr::AudioRoute &in, const int posY) {
    std::unique_ptr<Button> extint;
    if(isNew) 
        extint = std::make_unique<Button>(this, "EXT");
    else 
        extint = std::make_unique<Button>(this, in._sourceType == slr::AudioRoute::Type::EXT ? "EXT" : "INT");
    extint->setSize(Layout::ROUTE_CTL_LEFT_EXT_W, Layout::ROUTE_CTL_LEFT_EXT_H);
    extint->setPos(Layout::ROUTE_CTL_LEFT_EXT_X, Layout::ROUTE_CTL_LEFT_EXT_Y+posY);
    if(isNew) {
        extint->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
            std::string text = this->_nextInput._extint->text();
            if(text.compare("EXT") == 0) {
                this->_nextInput._extint->setText("INT");
                //fill drop
                std::vector<slr::AudioUnitView*> list = slr::ProjectView::getProjectView().unitList();
                //sort out self
                if(auto it = std::find_if(list.begin(), list.end(), [cmpid = this->_currentId](const slr::AudioUnitView *u) {
                    return u->id() == cmpid;
                }); it != list.end()) {
                    list.erase(it);
                }
                std::vector<std::string> names;
                for(auto u : list) {
                    names.push_back(u->name());
                }
                this->_nextInput._dropdown->setItems(names);
                this->_nextInput._dropdown->setSelected(
                    names.size() != 0 ? names.at(0) : NONE_STRING);
            } else {
                this->_nextInput._extint->setText("EXT");
                std::vector<std::string> items;
                items.push_back(std::string("Mono Left"));
                items.push_back(std::string("Mono Right"));
                items.push_back(std::string("Stereo"));
                this->_nextInput._dropdown->setItems(items);
                this->_nextInput._dropdown->setSelected(
                    items.size() != 0 ? items.at(0) : NONE_STRING);
            }
            return true;
        });
    }
    
    std::unique_ptr<DropDown> drop = std::make_unique<DropDown>(this);
    drop->setSize(Layout::ROUTE_CTL_LEFT_DD_W, Layout::ROUTE_CTL_LEFT_DD_H); 
    drop->setPos(Layout::ROUTE_CTL_LEFT_DD_X, Layout::ROUTE_CTL_LEFT_DD_Y+posY);
    if(isNew) {
        // slr::DriverView *dri = slr::DriverView::driverView(); -> fetch driver name
        std::vector<std::string> items;
        items.push_back(std::string("Mono Left"));
        items.push_back(std::string("Mono Right"));
        items.push_back(std::string("Stereo"));
        drop->setItems(items);
        drop->setSelected(items.size() != 0 ? items.at(0) : NONE_STRING);
    } else {
        std::string tmptext;
        if(in._sourceType == slr::AudioRoute::Type::EXT) {
            tmptext = "Audio Driver";
        } else {
            slr::AudioUnitView * au = slr::ProjectView::getProjectView().getUnitById(in._sourceId);
            tmptext = au->name();
        }

        drop->setSelected(tmptext);
        drop->disableButton();
    }

    std::unique_ptr<Button> map = std::make_unique<Button>(this, "MAP");
    map->setPos(Layout::ROUTE_CTL_LEFT_MAP_X, Layout::ROUTE_CTL_LEFT_MAP_Y+posY);
    map->setSize(Layout::ROUTE_CTL_LEFT_MAP_W, Layout::ROUTE_CTL_LEFT_MAP_H);

    std::unique_ptr<Button> addremove = std::make_unique<Button>(this, isNew ? LV_SYMBOL_PLUS : LV_SYMBOL_MINUS);
    addremove->setPos(Layout::ROUTE_CTL_LEFT_ADD_X, Layout::ROUTE_CTL_LEFT_ADD_Y+posY);
    addremove->setSize(Layout::ROUTE_CTL_LEFT_ADD_W, Layout::ROUTE_CTL_LEFT_ADD_H);
    if(isNew) {
        addremove->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
            this->newRoute(true);
            //update here? or... where?
            return true;
        });
    }
    // addremove->setCallback([this, isNew]() {
    //     if(isNew) {
    //         //create new route
    //     } else {
    //         //delete current route
    //     }
    // });

    if(isNew) {
        _nextInput._extint.swap(extint);
        _nextInput._dropdown.swap(drop);
        _nextInput._channelMap.swap(map);
        _nextInput._addRemoveButton.swap(addremove);
    } else { 
        Route r;
        r._extint = std::move(extint);
        r._dropdown = std::move(drop);
        r._channelMap = std::move(map);
        r._addRemoveButton = std::move(addremove);
        _inputs.push_back(std::move(r));
    }
}

void RouteManager::AudioTab::addAsOutput(bool isNew, const slr::AudioRoute &out, const int posY) {
    std::unique_ptr<Button> extint;
    if(isNew) 
        extint = std::make_unique<Button>(this, "EXT");
    else 
        extint = std::make_unique<Button>(this, out._targetType == slr::AudioRoute::Type::EXT ? "EXT" : "INT");
    extint->setSize(Layout::ROUTE_CTL_RIGHT_EXT_W, Layout::ROUTE_CTL_RIGHT_EXT_H);
    extint->setPos(Layout::ROUTE_CTL_RIGHT_EXT_X, Layout::ROUTE_CTL_RIGHT_EXT_Y+posY);
    if(isNew) {
        extint->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
            std::string text = this->_nextOutput._extint->text();
            if(text.compare("EXT") == 0) {
                this->_nextOutput._extint->setText("INT");
                //fill drop
                std::vector<slr::AudioUnitView*> list = slr::ProjectView::getProjectView().unitList();
                if(auto it = std::find_if(list.begin(), list.end(), [cmpid = this->_currentId](const slr::AudioUnitView *u) {
                    return u->id() == cmpid;
                }); it != list.end()) {
                    list.erase(it);
                }
                std::vector<std::string> names;
                for(auto u : list) {
                    names.push_back(u->name());
                }
                this->_nextOutput._dropdown->setItems(names);
                this->_nextOutput._dropdown->setSelected(
                    names.size() != 0 ? names.at(0) : NONE_STRING);
            } else {
                this->_nextOutput._extint->setText("EXT");
                std::vector<std::string> items;
                items.push_back(std::string("Mono Left"));
                items.push_back(std::string("Mono Right"));
                items.push_back(std::string("Stereo"));
                this->_nextOutput._dropdown->setItems(items);
                this->_nextOutput._dropdown->setSelected(items.at(0));
            }
            return true;
        }); 
    }

    std::unique_ptr<DropDown> drop = std::make_unique<DropDown>(this);
    drop->setSize(Layout::ROUTE_CTL_RIGHT_DD_W, Layout::ROUTE_CTL_RIGHT_DD_H);
    drop->setPos(Layout::ROUTE_CTL_RIGHT_DD_X, Layout::ROUTE_CTL_RIGHT_DD_Y+posY);
    
    if(isNew) {
        // slr::DriverView *dri = slr::DriverView::driverView(); -> fetch driver name
        std::vector<std::string> items;
        items.push_back(std::string("Mono Left"));
        items.push_back(std::string("Mono Right"));
        items.push_back(std::string("Stereo"));
        drop->setItems(items);
        drop->setSelected(items.size() != 0 ? items.at(0) : NONE_STRING);
    } else {
        std::string tmptext;

        if(out._targetType == slr::AudioRoute::Type::EXT) {
            tmptext = "Driver";
        } else {
            slr::AudioUnitView * au = slr::ProjectView::getProjectView().getUnitById(out._targetId);
            tmptext = au->name();
        }
        drop->setSelected(tmptext);
        drop->disableButton();
    }

    std::unique_ptr<Button> map = std::make_unique<Button>(this, "Map");
    map->setSize(Layout::ROUTE_CTL_RIGHT_MAP_W, Layout::ROUTE_CTL_RIGHT_MAP_H);
    map->setPos(Layout::ROUTE_CTL_RIGHT_MAP_X, Layout::ROUTE_CTL_RIGHT_MAP_Y+posY);
    map->setFont(&DEFAULT_FONT);

    std::unique_ptr<Button> addremove = std::make_unique<Button>(this, isNew ? LV_SYMBOL_PLUS : LV_SYMBOL_MINUS);
    addremove->setSize(Layout::ROUTE_CTL_RIGHT_ADD_W, Layout::ROUTE_CTL_RIGHT_ADD_H);
    addremove->setPos(Layout::ROUTE_CTL_RIGHT_ADD_X, Layout::ROUTE_CTL_RIGHT_ADD_Y+posY);
    if(isNew) {
        addremove->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
            this->newRoute(false);
            //update here? or... where?
            return true;
        });
    }
    // addremove->setCallback([this, isNew]() {
    //     if(isNew) {
    //         //add new route
    //     } else {
    //         //remove route
    //     }
    // });

    if(isNew) {
        _nextOutput._extint.swap(extint);
        _nextOutput._dropdown.swap(drop);
        _nextOutput._channelMap.swap(map);
        _nextOutput._addRemoveButton.swap(addremove);
    } else {
        Route r;
        r._extint = std::move(extint);
        r._dropdown = std::move(drop);
        r._channelMap = std::move(map);
        r._addRemoveButton = std::move(addremove);
        _outputs.push_back(std::move(r));
    }
}

void RouteManager::AudioTab::newRoute(bool isInput) {
    slr::AudioRoute newroute;
    for(int i=0; i<32; ++i) {
        newroute._channelMap[i] = -1;
    }
    if(isInput) {            
        const std::string io = _nextInput._extint->text();
        if(io.compare("EXT") == 0) {
            newroute._sourceType = slr::AudioRoute::Type::EXT;
            newroute._sourceId = 0;
                
            const std::string id = _nextInput._dropdown->selectedItem();
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
            const std::string name = _nextInput._dropdown->selectedItem();
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
        newroute._targetId = _currentId;
    } else {
        //output thing
        const std::string io = _nextOutput._extint->text();
        if(io.compare("EXT") == 0) {
            newroute._targetType = slr::AudioRoute::Type::EXT;
            newroute._targetId = 0;

            const std::string id = _nextOutput._dropdown->selectedItem();
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
            const std::string name = _nextOutput._dropdown->selectedItem();
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

        newroute._sourceId = _currentId;
        newroute._sourceType = slr::AudioRoute::Type::INT;
    }

    auto action = std::make_unique<slr::Actions::AddNewAudioRoute>();
    action->route = newroute;
    action->finished = [ptr = _rm]() {
        UIControls::DisplayTask([ptr]() {
            ptr->liveUpdate();
        });
    };
    slr::EmitAction(std::move(action));
}

RouteManager::MidiTab::MidiTab(BaseWidget *parent, const slr::ID &id, RouteManager *rm) 
    : BaseWidget(parent, true),
    _currentId(id),
    _rm(rm)
{
    setPos(0, 150);
    setSize(Layout::ROUTE_MANAGER_WIDTH, Layout::ROUTE_MANAGER_HEIGHT-150);
    // setColor(lv_color_hex(0x00ac00));
    addStyle(&Style::PopupDefault);

    _inputs.reserve(5);
    _outputs.reserve(5);
}

RouteManager::MidiTab::~MidiTab() {
    forcedClose();
}

void RouteManager::MidiTab::update() {
    const std::vector<slr::MidiRoute> & routes = slr::ProjectView::getProjectView().midiRoutes();
    for(const slr::MidiRoute &r : routes) {
        if(r._sourceId != _currentId && r._targetId != _currentId) continue;
        
         if(r._targetId == _currentId && r._targetType == slr::MidiRoute::Type::INT) {
            int posy = Layout::ROUTE_LINE_MARGIN + 
                        (Layout::ROUTE_LINE_HEIGHT*_inputs.size()) +
                        (Layout::ROUTE_LINE_MARGIN*_inputs.size());
            addAsInput(false, r, posy);
        } else if(r._sourceId == _currentId && r._sourceType == slr::MidiRoute::Type::INT) {
            int posy = Layout::ROUTE_LINE_MARGIN + 
                        (Layout::ROUTE_LINE_HEIGHT*_outputs.size()) +
                        (Layout::ROUTE_LINE_MARGIN*_outputs.size());
            addAsOutput(false, r, posy);
        }
    }
    
    int posy = Layout::ROUTE_LINE_MARGIN + 
                (Layout::ROUTE_LINE_HEIGHT*_inputs.size()) +
                (Layout::ROUTE_LINE_MARGIN*_inputs.size());
    slr::MidiRoute dummy;
    addAsInput(true, dummy, posy);

    posy = Layout::ROUTE_LINE_MARGIN + 
            (Layout::ROUTE_LINE_HEIGHT*_outputs.size()) +
            (Layout::ROUTE_LINE_MARGIN*_outputs.size());
    addAsOutput(true, dummy, posy);
}

void RouteManager::MidiTab::forcedClose() {
    _inputs.clear();
    _outputs.clear();

    _nextInput.reset();
    _nextOutput.reset();
}

void RouteManager::MidiTab::addAsInput(bool isNew, const slr::MidiRoute &in, const int posY) {
    std::unique_ptr<Button> extint;
    if(isNew) 
        extint = std::make_unique<Button>(this, "EXT");
    else 
        extint = std::make_unique<Button>(this, in._sourceType == slr::MidiRoute::Type::EXT ? "EXT" : "INT");

    extint->setSize(Layout::ROUTE_CTL_LEFT_EXT_W, Layout::ROUTE_CTL_LEFT_EXT_H);
    extint->setPos(Layout::ROUTE_CTL_LEFT_EXT_X, Layout::ROUTE_CTL_LEFT_EXT_Y+posY);
    if(isNew) {
        extint->tapCallback([this](const GestLib::TapGesture &tap) -> bool{
            std::string text = this->_nextInput._extint->text();
            if(text.compare("EXT") == 0) {
                this->_nextInput._extint->setText("INT");
                //fill drop
                std::vector<slr::AudioUnitView*> list = slr::ProjectView::getProjectView().unitList();
                //remove self
                if(auto it = std::find_if(list.begin(), list.end(), [cmpid = this->_currentId](const slr::AudioUnitView * u) {
                    return u->id() == cmpid;
                }); it != list.end()) {
                    list.erase(it);
                }
                std::vector<std::string> names;
                for(auto u : list) {
                    names.push_back(u->name());
                }
                this->_nextInput._dropdown->setItems(names);
                this->_nextInput._dropdown->setSelected(
                    names.size() != 0 ? names.at(0) : NONE_STRING);
            } else {
                this->_nextInput._extint->setText("EXT");
                std::vector<std::string> items;
                const std::vector<std::unique_ptr<slr::MidiPort>> & activePorts = slr::ControlEngine::midiController()->activePorts();
                for(auto &p : activePorts) {
                    if(p->inputOpened())
                        items.push_back(p->_ownerSubdev->_inputName);
                }
                
                this->_nextInput._dropdown->setItems(items);
                this->_nextInput._dropdown->setSelected(
                    items.size() != 0 ? items.at(0) : NONE_STRING);
            }
            return true;
        });
    }

    std::unique_ptr<DropDown> drop = std::make_unique<DropDown>(this);
    drop->setSize(Layout::ROUTE_CTL_LEFT_DD_W, Layout::ROUTE_CTL_LEFT_DD_H);
    drop->setPos(Layout::ROUTE_CTL_LEFT_DD_X, Layout::ROUTE_CTL_LEFT_DD_Y+posY);
    if(isNew) {
        const std::vector<std::unique_ptr<slr::MidiPort>> & activePorts = slr::ControlEngine::midiController()->activePorts();    
    
        std::vector<std::string> items;
        for(auto &p : activePorts) {
            if(p->inputOpened())
                items.push_back(p->_ownerSubdev->_inputName);
        }
        drop->setItems(items);
        drop->setSelected(items.size() != 0 ? items.at(0) : NONE_STRING);
    } else {
        const std::vector<std::unique_ptr<slr::MidiPort>> & activePorts = slr::ControlEngine::midiController()->activePorts();    
    
        std::string tmptext = "error";
        if(in._sourceType == slr::MidiRoute::Type::EXT) {
            for(auto &p : activePorts) {
                if(!p->isOpened()) continue;
                if(p->id() == in._sourceId && p->inputOpened()) {
                    tmptext = p->_ownerSubdev->_inputName;
                    break;
                }
            }
        } else {
            slr::AudioUnitView * view = slr::ProjectView::getProjectView().getUnitById(in._sourceId);
            tmptext = view->name();
        }
        drop->setSelected(tmptext);
        drop->disableButton();
    }

    std::unique_ptr<Button> map = std::make_unique<Button>(this, "Map");
    map->setSize(Layout::ROUTE_CTL_LEFT_MAP_W, Layout::ROUTE_CTL_LEFT_MAP_H);
    map->setPos(Layout::ROUTE_CTL_LEFT_MAP_X, Layout::ROUTE_CTL_LEFT_MAP_Y+posY);
    
    std::unique_ptr<Button> addremove = std::make_unique<Button>(this, isNew ? LV_SYMBOL_PLUS : LV_SYMBOL_MINUS);
    addremove->setSize(Layout::ROUTE_CTL_LEFT_ADD_W, Layout::ROUTE_CTL_LEFT_ADD_H);
    addremove->setPos(Layout::ROUTE_CTL_LEFT_ADD_X, Layout::ROUTE_CTL_LEFT_ADD_Y+posY);
    if(isNew) {
        addremove->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
            this->newRoute(true);
            return true;
        });
    } else {
        addremove->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
            LOG_INFO("add Remove Midi Route event");
            return true;
        });
    }

    if(isNew) {
        _nextInput._extint.swap(extint);
        _nextInput._dropdown.swap(drop);
        _nextInput._channelMap.swap(map);
        _nextInput._addRemoveButton.swap(addremove); 
    } else { 
        Route r;
        r._extint = std::move(extint);
        r._dropdown = std::move(drop);
        r._channelMap = std::move(map);
        r._addRemoveButton = std::move(addremove);
        _inputs.push_back(std::move(r)); 
    }
}

void RouteManager::MidiTab::addAsOutput(bool isNew, const slr::MidiRoute &out, const int posY) {
    Route r;
    
    std::unique_ptr<Button> extint = nullptr;
    if(isNew) 
        extint = std::make_unique<Button>(this, "EXT");
    else 
        extint = std::make_unique<Button>(this, out._targetType == slr::MidiRoute::Type::EXT ? "EXT" : "INT");
    extint->setSize(Layout::ROUTE_CTL_RIGHT_EXT_W, Layout::ROUTE_CTL_RIGHT_EXT_H);
    extint->setPos(Layout::ROUTE_CTL_RIGHT_EXT_X, Layout::ROUTE_CTL_RIGHT_EXT_Y+posY);
    if(isNew) {
        extint->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
            std::string text = this->_nextOutput._extint->text();
            if(text.compare("EXT") == 0) {
                this->_nextOutput._extint->setText("INT");
                //fill drop
                std::vector<slr::AudioUnitView*> list = slr::ProjectView::getProjectView().unitList();
                if(auto it = std::find_if(list.begin(), list.end(), [cmpid = this->_currentId](const slr::AudioUnitView *u) {
                    return u->id() == cmpid;
                }); it != list.end()) {
                    list.erase(it);
                }
                std::vector<std::string> names;
                for(auto u : list) {
                    names.push_back(u->name());
                }
                this->_nextOutput._dropdown->setItems(names);
                this->_nextOutput._dropdown->setSelected(
                    names.size() != 0 ? names.at(0) : NONE_STRING);
            } else {
                this->_nextOutput._extint->setText("EXT");
                std::vector<std::string> items;
                const std::vector<std::unique_ptr<slr::MidiPort>> & activePorts = slr::ControlEngine::midiController()->activePorts();
                for(auto &p : activePorts) {
                    if(p->outputOpened())
                        items.push_back(p->_ownerSubdev->_outputName);
                }
                this->_nextOutput._dropdown->setItems(items);
                this->_nextOutput._dropdown->setSelected(
                    items.size() != 0 ? items.at(0) : NONE_STRING);
            }
            return true;
        });
    }

    std::unique_ptr<DropDown> drop = std::make_unique<DropDown>(this);
    drop->setPos(Layout::ROUTE_CTL_RIGHT_DD_X, Layout::ROUTE_CTL_RIGHT_DD_Y+posY);
    drop->setSize(Layout::ROUTE_CTL_RIGHT_DD_W, Layout::ROUTE_CTL_RIGHT_DD_H);
    if(isNew) {
        const std::vector<std::unique_ptr<slr::MidiPort>> & activePorts = slr::ControlEngine::midiController()->activePorts();    
    
        std::vector<std::string> items;
        for(auto &p : activePorts) {
            if(p->outputOpened())
                items.push_back(p->_ownerSubdev->_outputName);
        }
        drop->setItems(items);
        drop->setSelected(items.size() != 0 ? items.at(0) : NONE_STRING);
    } else {
        const std::vector<std::unique_ptr<slr::MidiPort>> & activePorts = slr::ControlEngine::midiController()->activePorts();  
        
        std::string tmptext = "error";
        if(out._targetType == slr::MidiRoute::Type::EXT) {
            for(auto &p : activePorts) {
                if(!p->isOpened()) continue;
                if(p->id() == out._targetId && p->outputOpened()) {
                    tmptext = p->_ownerSubdev->_outputName;
                    break;
                }
            }

        } else {
            slr::AudioUnitView * view = slr::ProjectView::getProjectView().getUnitById(out._targetId);
            tmptext = view->name();
        }
        drop->setSelected(tmptext);
        drop->disableButton();
    }

    std::unique_ptr<Button> map = std::make_unique<Button>(this, "Map");
    map->setSize(Layout::ROUTE_CTL_RIGHT_MAP_W, Layout::ROUTE_CTL_RIGHT_MAP_H);
    map->setPos(Layout::ROUTE_CTL_RIGHT_MAP_X, Layout::ROUTE_CTL_RIGHT_MAP_Y+posY);

    std::unique_ptr<Button> addremove = std::make_unique<Button>(this, isNew ? LV_SYMBOL_PLUS : LV_SYMBOL_MINUS);
    addremove->setSize(Layout::ROUTE_CTL_RIGHT_ADD_W, Layout::ROUTE_CTL_RIGHT_ADD_H);
    addremove->setPos(Layout::ROUTE_CTL_RIGHT_ADD_X, Layout::ROUTE_CTL_RIGHT_ADD_Y+posY);
    addremove->setFont(&DEFAULT_FONT);
    if(isNew) {
        addremove->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
            this->newRoute(false);
            return true;
            //update here? or... where?
        });
    } else {
        addremove->tapCallback([this](const GestLib::TapGesture &tap) -> bool {
            LOG_INFO("add Remove Midi Route event");
            return true;
        });
    }
    if(isNew) {
        _nextOutput._extint.swap(extint);
        _nextOutput._dropdown.swap(drop);
        _nextOutput._channelMap.swap(map);
        _nextOutput._addRemoveButton.swap(addremove);
    } else { 
        Route r;
        r._extint = std::move(extint);
        r._dropdown = std::move(drop);
        r._channelMap = std::move(map);
        r._addRemoveButton = std::move(addremove);
        _outputs.push_back(std::move(r)); 
    }
}

void RouteManager::MidiTab::newRoute(bool isInput) {
    const std::vector<std::unique_ptr<slr::MidiPort>> & activePorts = slr::ControlEngine::midiController()->activePorts();
    slr::MidiRoute newroute;
    newroute._sourceChannel = 0;
    newroute._targetChannel = 0;

    if(isInput) {            
        //input thing
        const std::string io = _nextInput._extint->text();
        if(io.compare("EXT") == 0) {
            slr::ID srcid = 0;
            bool found = false;
            const std::string name = _nextInput._dropdown->selectedItem();
            for(auto &p : activePorts) {
                if(name.compare(p->_ownerSubdev->_inputName) == 0) {
                    srcid = p->id();
                    found = true;
                    break;
                }
            }

            if(!found) {
                LOG_WARN("Failed to find port");
                return;
            }

            newroute._sourceId = srcid;
            newroute._sourceType = slr::MidiRoute::Type::EXT;
        } else {
            //internal
            const std::string name = _nextInput._dropdown->selectedItem();
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
                LOG_WARN("Failed to find unit");
                return;
            }

            newroute._sourceId = id;
            newroute._sourceType = slr::MidiRoute::Type::INT;
        }

        newroute._targetId = _currentId;
        newroute._targetType = slr::MidiRoute::Type::INT;
    } else {
        //output thing
        const std::string io = _nextOutput._extint->text();
        if(io.compare("EXT") == 0) {
            //external
            slr::ID srcid = 0;
            const std::string name = _nextOutput._dropdown->selectedItem();
            for(auto &p : activePorts) {
                if(name.compare(p->_ownerSubdev->_outputName) == 0) {
                    srcid = p->id();
                    break;
                }
            }

            if(srcid == 0) {
                LOG_WARN("Failed to find port");
                return;
            }

            newroute._targetId = srcid;
            newroute._targetType = slr::MidiRoute::Type::EXT;
        } else {
            //internal
            const std::string name = _nextOutput._dropdown->selectedItem();
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
                LOG_WARN("Failed to find unit");
                return;
            }

            newroute._targetId = id;
            newroute._targetType = slr::MidiRoute::Type::INT;
        }

        newroute._sourceId = _currentId;
        newroute._sourceType = slr::MidiRoute::Type::INT;
    }

    
    auto action = std::make_unique<slr::Actions::AddNewMidiRoute>();
    action->route = newroute;
    action->finished = [ptr = _rm]() {
        UIControls::DisplayTask([ptr]() {
            ptr->liveUpdate();
        });
    };
    slr::EmitAction(std::move(action));
}

RouteManager::Route::Route(Route &&rhs) {
    _extint.swap(rhs._extint);
    _dropdown.swap(rhs._dropdown);
    _channelMap.swap(rhs._channelMap);
    _addRemoveButton.swap(rhs._addRemoveButton);
}
void RouteManager::Route::reset() {
    _extint.reset();
    _dropdown.reset();
    _channelMap.reset();
    _addRemoveButton.reset();
}

}