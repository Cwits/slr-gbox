// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/primitives/UnitUIBase.h"
#include "ui/display/primitives/UIContext.h"
#include "ui/display/primitives/Button.h"
#include "ui/display/layoutSizes.h"
#include "ui/display/RouteManager.h"
#include "ui/display/Timeline.h"
#include "ui/display/defaultStyles.h"
#include "ui/display/defaultColors.h"
#include "ui/display/primitives/Label.h"
#include "ui/display/primitives/FileView.h"
#include "ui/uiutility.h"

#include "snapshots/AudioUnitView.h"
#include "core/Events.h"

#include "logger.h"

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

bool UnitUIBase::update(UIContext * ctx) {
    
    for(auto clb : _externalUpdates) {
        clb();
    }

    //File check
    std::size_t viewSize = _view->_fileList._items.size();
    std::size_t uiSize = _viewItems.size();
    //create or delete
    if(viewSize != uiSize) {
        if(viewSize > uiSize) {
            //new items added
            std::vector<slr::ContainerItemView*> toAdd;
            for(std::size_t i=0; i<viewSize; ++i) {
                slr::ContainerItemView * item = _view->_fileList._items.at(i);
                bool found = false;
                for(std::size_t y=0; y<uiSize; ++y) {
                    if(_viewItems.at(y)->_item->_uniqueId == item->_uniqueId) {
                        found = true;
                    }
                }

                if(found) continue;
                toAdd.push_back(item);
            }

            //add to grid
            for(std::size_t i=0; i<toAdd.size(); ++i) {
                FileView * fw = new FileView(ctx->grid(), this, toAdd.at(i), _uictx);
                _viewItems.push_back(fw);
            }

            //add to moduleUI
        } else {
            //items removed
            std::vector<FileView*> toRemove;
            for(std::size_t i=0; i<uiSize; ++i) {
                FileView * item = _viewItems.at(i);
                bool found = false;
                for(std::size_t y=0; y<viewSize; ++y) {
                    if(item->id() == _view->_fileList._items.at(y)->_uniqueId) {
                        found = true;
                    }
                }

                if(found) continue;
                toRemove.push_back(item);
            }
            //need to erase from _viewItems container as well...
            for(std::size_t i=0; i<toRemove.size(); ++i) {
                slr::Events::FileUIRemoved e = {
                    .fileId = toRemove.at(i)->id(),
                    .targetId = _view->id()
                };
                slr::EmitEvent(e);
                for(std::size_t y=0; y<_viewItems.size(); ++y) {
                    if(_viewItems.at(y) == toRemove.at(i)) {
                        _viewItems.erase(_viewItems.begin()+y);
                    }
                }
                delete toRemove.at(i);
            }
        }

        ctx->_gridTimeline->updatePlayheadZ();
    }

    //update items positions(uiSize should be == viewSize)
    viewSize = _view->_fileList._items.size();
    uiSize = _viewItems.size();

    for(std::size_t i=0; i<uiSize; ++i) {
        FileView * fui = _viewItems.at(i);
        slr::ContainerItemView * fview = fui->_item;

        float xposition = UIUtility::frameToPixel(fview->_startPosition, _uictx->gridHorizontalZoom());
        fui->setPos(xposition, gridY());
        // fui->setPos()
    }

    return true;
}

bool UnitUIBase::destroy(UIContext * ctx) {
    for(FileView * fw : _viewItems) {
        delete fw;
    }
    return true;
}

int UnitUIBase::gridY() { 
    return lv_obj_get_y(gridUI()->lvhost());
}

void UnitUIBase::setNudge(slr::frame_t nudge, const float horizontalZoom) {
    // slr::TimelineView & tl = slr::TimelineView::getTimelineView();
    // int pixPerBar = UIUtility::pixelPerBar(horizontalZoom);
    // float pixMoved = ( (float)nudge / tl.framesPerBar() ) - ( (int)nudge/tl.framesPerBar() );
    // int startBar = (nudge/tl.framesPerBar());

    // std::size_t size = _viewItems.size();
    // for(std::size_t i=0; i<size; ++i) {
    //     FileView * item = _viewItems.at(i);
    //     // int cx = lv_obj_get_x(item->_canvas);
    //     int cx = item->getY();
    //     int cy = item->getY();
    //     int newx = -std::round((pixMoved+startBar)*pixPerBar);
    //     // lv_obj_set_x(item->_canvas, newx);
    //     item->setPos(newx, cy);
    // }
}

void UnitUIBase::updatePosition(int x, int y) {
    lv_obj_set_pos(gridUI()->lvhost(), x, y);
    // LOG_INFO("TODO: for items: update pos y");
    // for(FileView * fw : _viewItems) {
    //     int cx = fw->getX();
    //     // int cy = lv_obj_get_y(fw->_canvas);
    //     int cy = fw->getY();
    //     int newy = y;
    //     // lv_obj_set_y(fw->_canvas, y);
    //     fw->setPos(cx, newy);
    // }
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

}