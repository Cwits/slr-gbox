// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <cstdint>

namespace UI {

namespace LayoutDef {

/* General */
#define DEFAULT_FONT lv_font_montserrat_30
constexpr int BUTTON_SIZE = 80;

/* Main Window Defines */
constexpr int TOTAL_WIDTH = 1920;
constexpr int TOTAL_HEIGHT = 1080;

constexpr int DEFAULT_MARGIN = 20;

constexpr int TOP_PANEL_HEIGHT = ((TOTAL_HEIGHT/100) * 8);
constexpr int TOP_PANEL_WIDTH = TOTAL_WIDTH;

constexpr int BOTTOM_PANEL_HEIGHT = ((TOTAL_HEIGHT/100) * 10);
constexpr int BOTTOM_PANEL_WIDTH = TOTAL_WIDTH;
constexpr int BOTTOM_PANEL_X = 0;
constexpr int BOTTOM_PANEL_Y = (TOTAL_HEIGHT - BOTTOM_PANEL_HEIGHT);

constexpr int WORKSPACE_WIDTH = (TOTAL_WIDTH);
constexpr int WORKSPACE_HEIGHT = (TOTAL_HEIGHT - (TOP_PANEL_HEIGHT + BOTTOM_PANEL_HEIGHT));
constexpr int WORKSPACE_POSITION_X = 0;
constexpr int WORKSPACE_POSITION_Y = (TOP_PANEL_HEIGHT);

/* Grid View Defines */
constexpr int TRACK_CONTROL_PANEL_WIDTH = (WORKSPACE_WIDTH/100) * 20;
constexpr int TRACK_CONTROL_PANEL_HEIGHT = (WORKSPACE_HEIGHT);
constexpr int TRACK_CONTROL_PANEL_X = 0;
constexpr int TRACK_CONTROL_PANEL_Y = 0;
constexpr int GRID_WIDTH = (WORKSPACE_WIDTH - TRACK_CONTROL_PANEL_WIDTH);
constexpr int GRID_HEIGHT = (WORKSPACE_HEIGHT);
constexpr int GRID_X = (TRACK_CONTROL_PANEL_WIDTH);
constexpr int GRID_Y = 0;

/* Time Grid Defines */
//no defines for size and position because timegrid used in various scenarios
constexpr int TIMELINE_X = 0;
constexpr int TIMELINE_Y = 0;
constexpr int TIMELINE_WIDTH = GRID_WIDTH;
constexpr int TIMELINE_HEIGHT = 30;
constexpr int TIMELINE_LABEL_MARGIN = 30;
// constexpr int TIMELINE_LABEL_FONT_SIZE = 15;
constexpr int TIMELINE_LABEL_SIZE = 20;
constexpr int TIMELINE_LINE_Y = TIMELINE_HEIGHT+1;
constexpr int TIMELINE_LINE_WIDTH = 5;
constexpr int TIMELINE_LINE_HEIGHT = (GRID_HEIGHT - TIMELINE_LINE_Y)+30;

constexpr int TIMELINE_LOOP_HANDLE_W = BUTTON_SIZE-20;
constexpr int TIMELINE_LOOP_HANDLE_H = BUTTON_SIZE-20;

/* Timeline Popup */
constexpr int TIMELINE_POPUP_X = 500;
constexpr int TIMELINE_POPUP_Y = 400;
constexpr int TIMELINE_POPUP_W = 300;
constexpr int TIMELINE_POPUP_H = 400;

constexpr int TLPOP_APPLY_X = TIMELINE_POPUP_W - (BUTTON_SIZE + 10);
constexpr int TLPOP_APPLY_Y = TIMELINE_POPUP_H - (BUTTON_SIZE - 10);
// constexpr int TLPOP_BPM_X = (TIMELINE_POPUP_W/2)-50;
// constexpr int TLPOP_BPM_Y = 20;


/* Track Related */
#define TRACK_CONTROL_BUTTON_SIZE 50

constexpr int TRACK_HEIGHT = 130;
constexpr int TRACK_DISTANCE = 15;

inline int calcTrackY(std::size_t pos) {
    return TRACK_HEIGHT*pos+(TRACK_DISTANCE*pos) + TIMELINE_HEIGHT;
}

constexpr int TRACK_NAME_LABEL_X = 10;
constexpr int TRACK_NAME_LABEL_Y = 10;
constexpr int TRACK_NAME_LABEL_W = TRACK_CONTROL_PANEL_WIDTH-TRACK_NAME_LABEL_X;
// constexpr int TRACK_NAME_LABEL_H = 33;



/* Route Manager */
constexpr int ROUTE_MANAGER_WIDTH = (TOTAL_WIDTH - 200);
constexpr int ROUTE_MANAGER_HEIGHT = (TOTAL_HEIGHT - 100);
constexpr int ROUTE_MANAGER_X = 200/2;
constexpr int ROUTE_MANAGER_Y = 100/2;

/* Route Controls */
constexpr int ROUTE_LINE_HEIGHT = 70;
constexpr int ROUTE_LINE_MARGIN = 10;

constexpr int ROUTE_LINE_FIRST_ITEM = 90;

constexpr int ROUTE_CTL_LEFT_EXT_X = 25;
constexpr int ROUTE_CTL_LEFT_EXT_Y = ROUTE_LINE_FIRST_ITEM;
constexpr int ROUTE_CTL_LEFT_EXT_W = 60;
constexpr int ROUTE_CTL_LEFT_EXT_H = ROUTE_LINE_HEIGHT;

constexpr int ROUTE_CTL_LEFT_DD_X = 90;
constexpr int ROUTE_CTL_LEFT_DD_Y = ROUTE_LINE_FIRST_ITEM;
constexpr int ROUTE_CTL_LEFT_DD_W = 525;
constexpr int ROUTE_CTL_LEFT_DD_H = ROUTE_LINE_HEIGHT;

constexpr int ROUTE_CTL_LEFT_MAP_X = 620;
constexpr int ROUTE_CTL_LEFT_MAP_Y = ROUTE_LINE_FIRST_ITEM;
constexpr int ROUTE_CTL_LEFT_MAP_W = 90;
constexpr int ROUTE_CTL_LEFT_MAP_H = ROUTE_LINE_HEIGHT;

constexpr int ROUTE_CTL_LEFT_ADD_X = 715;
constexpr int ROUTE_CTL_LEFT_ADD_Y = ROUTE_LINE_FIRST_ITEM;
constexpr int ROUTE_CTL_LEFT_ADD_W = 90;
constexpr int ROUTE_CTL_LEFT_ADD_H = ROUTE_LINE_HEIGHT;


constexpr int ROUTE_CTL_RIGHT_EXT_X = 810+100;
constexpr int ROUTE_CTL_RIGHT_EXT_Y = ROUTE_LINE_FIRST_ITEM;
constexpr int ROUTE_CTL_RIGHT_EXT_W = 60;
constexpr int ROUTE_CTL_RIGHT_EXT_H = ROUTE_LINE_HEIGHT;

constexpr int ROUTE_CTL_RIGHT_DD_X = 880+100;
constexpr int ROUTE_CTL_RIGHT_DD_Y = ROUTE_LINE_FIRST_ITEM;
constexpr int ROUTE_CTL_RIGHT_DD_W = 525;
constexpr int ROUTE_CTL_RIGHT_DD_H = ROUTE_LINE_HEIGHT;

constexpr int ROUTE_CTL_RIGHT_MAP_X = 1410+100;
constexpr int ROUTE_CTL_RIGHT_MAP_Y = ROUTE_LINE_FIRST_ITEM;
constexpr int ROUTE_CTL_RIGHT_MAP_W = 90;
constexpr int ROUTE_CTL_RIGHT_MAP_H = ROUTE_LINE_HEIGHT;

constexpr int ROUTE_CTL_RIGHT_ADD_X = 1505+100;
constexpr int ROUTE_CTL_RIGHT_ADD_Y = ROUTE_LINE_FIRST_ITEM;
constexpr int ROUTE_CTL_RIGHT_ADD_W = 90;
constexpr int ROUTE_CTL_RIGHT_ADD_H = ROUTE_LINE_HEIGHT;


/* Browser */
#define BROWSER_ELEMENT_FONT lv_font_montserrat_46
constexpr int PARSED_PATH_X = 100;
constexpr int PARSED_PATH_Y = 5;
constexpr int PARSED_PATH_W = 1500;

constexpr int BROWSER_ELEMENT_MARGIN = 10;
constexpr int BROWSER_ELEMENT_ICON_X = 15;
constexpr int BROWSER_ELEMENT_TEXT_W = 1500;


/* Screen Keyboard */
constexpr int KEYBOARD_H = (TOTAL_HEIGHT/2)-100;
constexpr int KEYBOARD_W = TOTAL_WIDTH;
constexpr int KEYBOARD_X = 0;
constexpr int KEYBOARD_Y = (TOTAL_HEIGHT - KEYBOARD_H);

constexpr int KB_BUTTON_SIZE = BUTTON_SIZE;
constexpr int KB_BUTTON_MARGIN = 10;

constexpr int KB_TEXT_AREA_X = 0;
constexpr int KB_TEXT_AREA_Y = 0;
constexpr int KB_TEXT_AREA_W = TOTAL_WIDTH;
constexpr int KB_TEXT_AREA_H = 50;

constexpr int KB_BACKSPACE_W = KB_BUTTON_SIZE*2;
constexpr int KB_BACKSPACE_H = KB_BUTTON_SIZE;
constexpr int KB_BACKSPACE_X = KEYBOARD_W - KB_BACKSPACE_W - KB_BUTTON_MARGIN;
constexpr int KB_BACKSPACE_Y = KB_TEXT_AREA_H + DEFAULT_MARGIN;

constexpr int KB_ENTER_W = KB_BUTTON_SIZE*2;
constexpr int KB_ENTER_H = KB_BUTTON_SIZE;
constexpr int KB_ENTER_X = KEYBOARD_W - KB_ENTER_W - KB_BUTTON_MARGIN;
constexpr int KB_ENTER_Y = KB_TEXT_AREA_H + (DEFAULT_MARGIN*2) + KB_BUTTON_SIZE;

constexpr int KB_CLEAR_W = KB_BUTTON_SIZE*2;
constexpr int KB_CLEAR_H = KB_BUTTON_SIZE;

constexpr int KB_SPACE_W = KB_BUTTON_SIZE*2;
constexpr int KB_SPACE_H = KB_BUTTON_SIZE;

constexpr int KEYS_ROW_NUM = 10;
constexpr int KEYS_ROW_1 = 10;
constexpr int KEYS_ROW_2 = 9;
constexpr int KEYS_ROW_3 = 7;

/* File Popup */
constexpr int FILE_POPUP_W = 300;
constexpr int FILE_POPUP_H = 300;

constexpr int FILE_POP_DELETE_BTN_X = 10;
constexpr int FILE_POP_DELETE_BTN_Y = 10;


} //namespace Layout Def

} //namespace UI