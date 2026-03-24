// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/primitives/UnitUIBase.h"
#include "ui/display/primitives/UIContext.h"
#include "ui/display/primitives/Button.h"
#include "ui/display/primitives/Label.h"
#include "ui/display/primitives/FileView.h"

#include "ui/display/layoutSizes.h"
#include "ui/display/RouteManager.h"
#include "ui/display/Timeline.h"
#include "ui/display/defaultStyles.h"
#include "ui/display/defaultColors.h"
#include "ui/uiutility.h"

#include "snapshots/AudioUnitView.h"

#include "core/utility/helper.h"
#include "core/Events.h"

#include "logger.h"

#include <string>

namespace UI {

UnitUIBase::UnitUIBase(slr::AudioUnitView * view, UIContext * uictx) : 
    _view(view),
    _uictx(uictx)
{
    _canLoadFiles = true;
}

UnitUIBase::~UnitUIBase() {

}

const slr::ID UnitUIBase::id() const {
    return _view->id();
}

const slr::Color & UnitUIBase::color() const {
    return _view->color();
}

bool UnitUIBase::destroy(UIContext * ctx) {
    return true;
}

UnitControlPopup::UnitControlPopup(BaseWidget * parent, UIContext * const uictx) :
    Popup(parent, uictx) {
    setSize(300, 300);
    setPos(LayoutDef::TRACK_CONTROL_PANEL_WIDTH-100, 150);

    _deleteBtn = new Button(this, LV_SYMBOL_TRASH);
    _deleteBtn->setPos(10, 10);
    _deleteBtn->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _deleteBtn->setFont(&DEFAULT_FONT);
    _deleteBtn->setCallback([this]() {
        // std::cout << "Delete track: " << (int)_track->id() << std::endl;
        LOG_INFO("Delete track: %i", _currentUnit->id());
        slr::Events::DeleteModule e = {
            .targetId = _currentUnit->id()
        };
        slr::EmitEvent(e);

        this->_uictx->setLastSelected(nullptr);
        this->_uictx->_popManager->disableUnitControl();
    });

    _routeManagerBtn = new Button(this, "Routes");
    _routeManagerBtn->setPos(LayoutDef::BUTTON_SIZE+20, 10);
    _routeManagerBtn->setFont(&DEFAULT_FONT);
    _routeManagerBtn->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _routeManagerBtn->setCallback([this]() {
        // LOG_INFO("Call the manager!!! track %i", _currentTrack->id());
        this->_uictx->_popManager->disableUnitControl();
        this->_uictx->_popManager->enableRouteManager(this->_currentUnit->id());
    });
}

UnitControlPopup::~UnitControlPopup() {
    delete _deleteBtn;
    delete _routeManagerBtn;
}


DefaultGridUI::DefaultGridUI(BaseWidget * parent, UnitUIBase *base) 
    : BaseWidget(parent, true, true), 
    _uibase(base)
{
    _flags.isDoubleTap = true;

    setSize(LayoutDef::TRACK_CONTROL_PANEL_WIDTH, LayoutDef::TRACK_HEIGHT);
    int y = LayoutDef::calcTrackY(_uibase->uictx()->_unitsUI.size());
    setPos(0, y);

    lv_obj_set_style_bg_color(lvhost(),
                    lv_color_make(_uibase->view()->color().r, 
                                    _uibase->view()->color().g, 
                                    _uibase->view()->color().b), 0);

    _lblName = std::make_unique<Label>(this, _uibase->view()->name().c_str());
    _lblName->setSize(LayoutDef::TRACK_NAME_LABEL_W, lv_font_get_line_height(&DEFAULT_FONT));
    _lblName->setPos(LayoutDef::TRACK_NAME_LABEL_X, LayoutDef::TRACK_NAME_LABEL_Y);
    _lblName->setFont(&DEFAULT_FONT);
    _lblName->setTextColor(lv_color_hex(0xffffff));
    _lblName->setHoldCallback([this]() {
        this->_uibase->uictx()->_popManager->enableKeyboard(
            this->_uibase->view()->name(), 
            [this](const std::string & text) {
                LOG_INFO("New track name: %s for id: %d", text.c_str(), this->_uibase->view()->id());
                this->_uibase->nonconstview()->setName(text);
                // this->_lblName->setText(text);
            }
        );
    });

    _lblVolume = std::make_unique<Label>(this, std::to_string(_uibase->view()->volume()));
    _lblVolume->setSize(80, lv_font_get_line_height(&DEFAULT_FONT));
    _lblVolume->setPos(280, LayoutDef::TRACK_NAME_LABEL_Y);
    _lblVolume->setFont(&DEFAULT_FONT);
    _lblVolume->setTapCallback([this]() {
        this->_uibase->uictx()->_popManager->enableKeyboard(
            std::to_string(this->_uibase->view()->volume()),
            [this](const std::string & text) {
                float vol = std::stof(text);
                LOG_WARN("No check for volume!");
                LOG_INFO("Setting volume to: %f", vol);
                slr::Events::SetParameter e = {
                    e.targetId = this->_uibase->view()->id(),
                    e.parameterId = this->_uibase->view()->volumeId(),
                    e.value = vol
                };
                slr::EmitEvent(e);
                // this->_lblVolume->setText(text);
            }
        );
    });

    int posx = 10;
    int posy = 50;
    _btnMute = std::make_unique<Button>(this, "M");
    _btnMute->setPos(posx, posy);
    _btnMute->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnMute->setFont(&lv_font_montserrat_40);
    _btnMute->setCallback([this]() {
        // std::cout << "Mute track: " << (int)_track->id() << " parid: " << _track->muteId() << std::endl;
        slr::Events::SetParameter e = {
            .targetId = _uibase->view()->id(),
            .parameterId = _uibase->view()->muteId(),
            .value = (_uibase->view()->mute() ? slr::boolToFloat(false) : slr::boolToFloat(true))
        };
        slr::EmitEvent(e);
    });

    posx += (LayoutDef::DEFAULT_MARGIN + LayoutDef::BUTTON_SIZE);
    _btnSolo = std::make_unique<Button>(this, "S");
    _btnSolo->setPos(posx, posy);
    _btnSolo->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _btnSolo->setFont(&lv_font_montserrat_40);
    _btnSolo->setCallback([this]() {
        // std::cout << "Solo track: " << (int)_track->id() << " parid: " << "1" << std::endl;
    });
    
    show();
}

DefaultGridUI::~DefaultGridUI() {

}

void DefaultGridUI::pollFileUpdate() {
    const slr::AudioUnitView * view = _uibase->view();
    uint64_t containerVersion = view->_clipContainer.version();
    UIContext * const uictx = _uibase->uictx();

    if(containerVersion != _fileContainerVersion) {
        _fileContainerVersion = containerVersion;
        //File check
        const std::vector<const slr::ClipItemView*> &items = view->_clipContainer.clips();
        std::size_t viewSize = items.size();
        std::size_t uiSize = _fileUIs.size(); 
        //create or delete
        if(viewSize != uiSize) {
            if(viewSize > uiSize) {
                //new items added
                std::vector<const slr::ClipItemView*> toAdd;
                for(std::size_t i=0; i<viewSize; ++i) {
                    const slr::ClipItemView * item = items.at(i); 
                    bool found = false;
                    for(std::size_t y=0; y<uiSize; ++y) {
                        if(_fileUIs.at(y)->_clipItem->id() == item->id()) {
                            found = true;
                        }
                    }

                    if(found) continue;
                    toAdd.push_back(item);
                }

                //add to grid
                for(std::size_t i=0; i<toAdd.size(); ++i) {
                    std::unique_ptr<FileView> fw = std::make_unique<FileView>(uictx->grid(), _uibase, toAdd.at(i), uictx);
                    _fileUIs.push_back(std::move(fw));
                }

                //add to moduleUI
            } else {
                //items removed
                std::vector<FileView*> toRemove;
                for(std::size_t i=0; i<uiSize; ++i) {
                    std::unique_ptr<FileView>& item = _fileUIs.at(i);
                    bool found = false;
                    for(std::size_t y=0; y<viewSize; ++y) {
                        if(item->id() == items.at(y)->id()) { 
                            found = true;
                        }
                    }

                    if(found) continue;
                    toRemove.push_back(item.get());
                }
                //need to erase from _viewItems container as well...
                for(std::size_t i=0; i<toRemove.size(); ++i) {
                    slr::Events::ClipUIRemoved e = {
                        .clipId = toRemove.at(i)->id(),
                        .unitId = view->id()
                    };
                    
                    for(std::size_t y=0; y<_fileUIs.size(); ++y) {
                        if(_fileUIs.at(y).get() == toRemove.at(i)) {
                            _fileUIs.erase(_fileUIs.begin()+y);
                            y = 0;
                        }
                    }
                    delete toRemove.at(i);

                    slr::EmitEvent(e);
                }
            }

            uictx->_gridTimeline->updatePlayheadZ();
        }
    }

    //file UI handle versions by themselves
    const std::vector<const slr::ClipItemView*> &items = view->_clipContainer.clips();
    std::size_t viewSize = items.size();
    std::size_t uiSize = _fileUIs.size(); 

    for(std::size_t i=0; i<uiSize; ++i) {
        std::unique_ptr<FileView>& fui = _fileUIs.at(i);

        fui->pollUIUpdate();
    }
}

void DefaultGridUI::pollUIUpdate() {
    pollFileUpdate();
    
    const slr::AudioUnitView * view = _uibase->view();
    if(isSameUIVersion(view->version())) return;

    if(view->mute()) {
        _btnMute->setColor(MUTE_ON_COLOR);
    } else {
        _btnMute->setColor(MUTE_OFF_COLOR);
    }

    // if(view->solo()) {
    //     //...
    // }

    _lblVolume->setText(std::to_string(view->volume()));
    _lblName->setText(view->name());
}

int DefaultGridUI::gridY() {
    return lv_obj_get_y(lvhost());
}

void DefaultGridUI::setNudge(slr::frame_t nudge, const float horizontalZoom) {
    slr::TimelineView & tl = slr::TimelineView::getTimelineView();
    int pixPerBar = UIUtility::pixelPerBar(horizontalZoom);
    float pixMoved = ( (float)nudge / tl.framesPerBar() ) - ( (int)nudge/tl.framesPerBar() );
    int startBar = (nudge/tl.framesPerBar());

    std::size_t size = _fileUIs.size();
    for(std::size_t i=0; i<size; ++i) {
        std::unique_ptr<FileView>& item = _fileUIs.at(i);
        int cx = item->getY();
        int cy = item->getY();
        int newx = -std::round((pixMoved+startBar)*pixPerBar);
        item->setPos(newx, cy);
    }
}

void DefaultGridUI::updatePosition(int x, int y) {
    lv_obj_set_pos(lvhost(), x, y);
    for(std::unique_ptr<FileView> & fw : _fileUIs) {
        int cx = fw->getX();
        int cy = fw->getY();
        int newy = y;
        fw->setPos(cx, newy);
    }
}

std::vector<FileView*> DefaultGridUI::fileList() {
    std::vector<FileView*> ret;

    for(std::unique_ptr<FileView> & fv : _fileUIs) {
        ret.push_back(fv.get());
    }

    return ret; 
}

bool DefaultGridUI::handleDoubleTap(GestLib::DoubleTapGesture & dt) {
    LOG_INFO("Double Tap on track controls %i", _uibase->view()->id());
    _uibase->uictx()->_popManager->enableUnitControl(_uibase, this);
    return true;
}



}