// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/Browser.h"
#include "ui/display/layoutSizes.h"
#include "ui/display/defaultStyles.h"
#include "ui/display/defaultColors.h"
#include "ui/display/primitives/UIContext.h"
#include "ui/display/primitives/DragContext.h"
#include "ui/display/primitives/Button.h"
#include "ui/display/primitives/Label.h"

#include "core/SettingsManager.h"

#include "logger.h"

#include <dirent.h>
#include <algorithm>

namespace UI {

const int FIRST_ITEM_Y = 10+LayoutDef::PARSED_PATH_Y+lv_font_get_line_height(&DEFAULT_FONT);
const int _lineHeight = lv_font_get_line_height(&BROWSER_ELEMENT_FONT)+3;

// const std::string DEFAULE_PATH = "/home/portablejoe/music/";

Browser::Browser(BaseWidget* parent, UIContext * const uictx) : View(parent, uictx) {
    setPos(LayoutDef::WORKSPACE_POSITION_X, LayoutDef::WORKSPACE_POSITION_Y);
    setSize(LayoutDef::WORKSPACE_WIDTH, LayoutDef::WORKSPACE_HEIGHT);
    lv_obj_add_style(_lvhost, &workspace, 0);
    lv_obj_set_style_bg_color(_lvhost, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);

    _flags.isTouchDown = true;
    _flags.isTouchUp = true;
    _flags.isTap = true;
    _flags.isDrag = true;
    _flags.isSwipe = true;

    int posx = parent->width()-LayoutDef::BUTTON_SIZE-LayoutDef::DEFAULT_MARGIN;
    _refresh = new Button(this, LV_SYMBOL_REFRESH);
    _refresh->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _refresh->setPos(posx, LayoutDef::DEFAULT_MARGIN);
    _refresh->setFont(&DEFAULT_FONT);
    _refresh->setCallback([this]() {
        this->parse();
    });

    posx -= (LayoutDef::BUTTON_SIZE+LayoutDef::DEFAULT_MARGIN);
    _dirUp = new Button(this, LV_SYMBOL_UP);
    _dirUp->setSize(LayoutDef::BUTTON_SIZE, LayoutDef::BUTTON_SIZE);
    _dirUp->setPos(posx, LayoutDef::DEFAULT_MARGIN);
    _dirUp->setFont(&DEFAULT_FONT);
    _dirUp->setCallback([this]() {
        this->goUp();
    });

    _lastPath = new Label(this, "");
    _lastPath->setSize(LayoutDef::PARSED_PATH_W, lv_font_get_line_height(&DEFAULT_FONT));
    _lastPath->setPos(LayoutDef::PARSED_PATH_X, LayoutDef::PARSED_PATH_Y);
    _lastPath->setFont(&DEFAULT_FONT);

    _lastTouched = nullptr;

    parse();
}

Browser::~Browser() {
    for(auto * el : _elements) {
        delete el;
    }

    delete _refresh;
    delete _dirUp;
}

void Browser::parse() {
    struct dirent *entry;
    if(_lastParsedPath.empty()) _lastParsedPath = slr::SettingsManager::getDefaultStorage();

    DIR * dir = opendir(_lastParsedPath.c_str());
    if(dir == nullptr) {
        LOG_ERROR("Browser: Failed to parse path %s", _lastParsedPath.c_str());
        return;
    }

    int subdirCount = 0;
    int fileCount = 0;
    _folders.clear();
    _files.clear();

    while((entry = readdir(dir)) != NULL) {
        std::string temp;
        temp.append(entry->d_name);
        if(temp == "." || temp =="..") continue;
        if(temp.at(0) == '.') continue;
        if(entry->d_type == 4) {
            _folders.push_back(temp);
            subdirCount++;
        } else if(entry->d_type == 8) {
            _files.push_back(temp);
            fileCount++;
        }
    }
    
    closedir(dir);   
    
    std::sort(_folders.begin(), _folders.end());
    std::sort(_files.begin(), _files.end());

    std::size_t s = _folders.size()+_files.size();
    for(auto * el : _elements) {
        delete el;
    }
    _elements.clear();
    _elements.reserve(s);

    int initY = FIRST_ITEM_Y;
    const int margin = 10;
    std::size_t subs = _folders.size();
    static std::string iconText;
    for(std::size_t i=0; i<s; ++i) {
        std::string & text = (i >= subs ? _files.at(i-subs) : _folders.at(i));
        
        iconText.clear();
        if(i < subs) {
            iconText = std::string(LV_SYMBOL_DIRECTORY);
        } else {
            //file
            if(text.substr(text.size()-4, text.size()) == ".wav") {
                iconText = std::string(LV_SYMBOL_AUDIO);
            } else if(text.substr(text.size()-6, text.size()) == ".slrpk") {
                iconText = std::string(LV_SYMBOL_IMAGE); 
            } else {
                iconText = std::string(LV_SYMBOL_FILE);
            }
            
        }

        element * el = new element();
        el->init(this, iconText, text, initY);
        _elements.push_back(el);

        initY += (_lineHeight+margin);
    }

    _lastPath->setText(_lastParsedPath);
}

void Browser::goUp() {
    std::string tmp = _lastParsedPath.substr(0, _lastParsedPath.find_last_of("/"));
    std::string newPath = _lastParsedPath.substr(0, tmp.find_last_of("/")+1);
    _lastParsedPath = newPath;
    parse();
}

bool Browser::handleTouchDown(GestLib::TouchDownEvent &down) {
    int notAbsX = down.x - getX();
    int notAbsY = down.y - getY();
    Label * lb = findLabel(notAbsX, notAbsY);

    if(lb) {
        lb->setTextColor(WHITE_COLOR);

        _lastTouched = lb;
    }

    return true;
}

bool Browser::handleTouchUp(GestLib::TouchUpEvent &up) {
    if(_lastTouched) {
        _lastTouched->setTextColor(BLACK_COLOR);
        _lastTouched = nullptr;
    } 

    return true;
}

bool Browser::handleTap(GestLib::TapGesture &tap) { 
    int notAbsX = tap.x - getX();
    int notAbsY = tap.y - getY();

    std::size_t size = _folders.size();
    if(notAbsY < (static_cast<int>(size)*(_lineHeight+10))+FIRST_ITEM_Y) { //lineheight + margin
        //it's folder
        Label * lb = findLabel(notAbsX, notAbsY);

        if(lb) {
            // std::string text;
            // text = lb->text();
            _lastParsedPath.append(lb->text());
            _lastParsedPath.append("/");

            //because otherwise we'll be checking garbage -> 
            //label most probably will be deleted at that point
            _uictx->cancleGesture(lb);
            _lastTouched = nullptr;
            parse();
        }
    } else {
        //it's file
    }

    return true;
}

bool Browser::handleDrag(GestLib::DragGesture & drag) {
    if(drag.state == GestLib::GestureState::Start) {
        LOG_INFO("Drag Start x: %d, y: %d", drag.x, drag.y);
        int notAbsX = drag.x - getX();
        int notAbsY = drag.y - getY();

        //find label under drag
        Label * lb = findLabel(notAbsX, notAbsY);

        if(lb == nullptr) {
            LOG_WARN("Failed to find label");
            return false;
        }
        // LOG_INFO("Found label: %s", lb->text().c_str());

        _dragndrop.clear();
        _dragndrop.append(_lastParsedPath);
        _dragndrop.append(lb->text());
        // _dragndrop = lb->text();

        DragContext & ctx = _uictx->_dragContext;
        ctx.origin = this;
        ctx.startX = drag.x;
        ctx.startY = drag.y;
        ctx.payload.type = DragPayload::DataType::FilePath;
        ctx.payload.filePath.path = &_dragndrop;
        ctx.dragOnGoing = true;

        /* 
            TODO: show selector that will allow to select where we want to drag.
            that means we have to make transferGesture to selector and than from selector
            make another transfer to destination
        */
        
        _uictx->transferGesture(_uictx->previousView(), GestLib::Gestures::Drag);

        // return true;
    } else if(drag.state == GestLib::GestureState::Move) {
        LOG_INFO("Drag Move x: %d, y: %d, dx: %d, dy: %d", drag.x, drag.y, drag.dx, drag.dy);
    } else if(drag.state == GestLib::GestureState::End) {
        LOG_INFO("Drag End x: %d, y: %d", drag.x, drag.y);
    }

    return true;
}

bool Browser::handleSwipe(GestLib::SwipeGesture &swipe) {
    if(swipe.state == GestLib::GestureState::Start) {

    } else if(swipe.state == GestLib::GestureState::Move) {
        std::size_t size = _elements.size();
        if(size == 0) return false;

        int totalHeight = static_cast<int>(size) * _lineHeight;
        if(totalHeight < (height()-FIRST_ITEM_Y)) return false;

        // LOG_INFO("Browser Swipe");
        int mod = swipe.dy*2;
        if(std::abs(swipe.dy) > 4) {
            //vertical move
            if(swipe.dy > 0) {
                //from top to bottom -> scroll down
                int y = _elements.at(0)->_text->getY();
                if(y >= FIRST_ITEM_Y) return true;
                
                int diff = FIRST_ITEM_Y - y;
                if(diff < mod) mod = diff;

                for(std::size_t i=0; i<size; ++i) {
                    element *el = _elements.at(i);
                    Label *lb = el->_text;

                    int cx = lb->getX();
                    int cy = lb->getY();
                    lb->setPos(cx, cy+mod);
                    lv_obj_set_y(el->_icon, cy+mod);
                }
            } else if(swipe.dy < 0) {
                //from bottom to top -> scroll up
                int y = _elements.at(size-1)->_text->getY();
                if(y <= (height()-FIRST_ITEM_Y)) return true;

                int diff = (height()-FIRST_ITEM_Y) - y;
                if(diff > mod) mod = diff;

                for(std::size_t i=0; i<size; ++i) {
                    element *el = _elements.at(i);
                    Label *lb = el->_text;

                    int cx = lb->getX();
                    int cy = lb->getY();
                    lb->setPos(cx, cy+mod);
                    lv_obj_set_y(el->_icon, cy+mod);
                }
            }
        }
    
    } else if(swipe.state == GestLib::GestureState::End) {

    }
    return true;
}

Label * Browser::findLabel(int x, int y) {
    std::size_t size = _elements.size();
    
    for(std::size_t i=0; i<size; ++i) {
        Label * lb = _elements.at(i)->_text;
        if(!lb->contains(x, y)) {
            continue;
        }

        return lb;
    }

    return nullptr;
}

Browser::element::~element() {
    lv_obj_delete(_icon); 
    delete _text;
}

void Browser::element::init(Browser *parent, std::string &iconText, std::string &labelText, int ypos) {
    lv_obj_t * icon = lv_label_create(parent->lvhost());
    lv_label_set_text(icon, iconText.c_str());
    lv_obj_set_size(icon, _lineHeight, _lineHeight);
    lv_obj_set_pos(icon, LayoutDef::BROWSER_ELEMENT_ICON_X, ypos);
    lv_obj_set_style_text_font(icon, &BROWSER_ELEMENT_FONT, 0);

    Label * lb = new Label(parent, labelText);
    lb->setPos(_lineHeight+LayoutDef::BROWSER_ELEMENT_ICON_X, ypos);
    lb->setSize(LayoutDef::BROWSER_ELEMENT_TEXT_W, _lineHeight);
    lb->setFont(&BROWSER_ELEMENT_FONT);
    lb->setTextPos(5, 0);
    //ugh... animation is heavy thing :/
    // lv_obj_set_size(lb->label(), BROWSER_ELEMENT_TEXT_W, _lineHeight);
    // lv_label_set_long_mode(lb->label(), LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
    // lv_obj_set_style_anim(lb->label(), &_browserElementAnimation, 0);
    lv_obj_add_style(lb->lvhost(), &browserElementStyle, 0);
        

    _icon = icon;
    _text = lb;
}

}