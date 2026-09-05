#pragma once

#define LABEL(ptr, x, y, w, h, text) \
    ptr = std::make_unique<Label>(this, text); \
    ptr->setPos(x, y); \
    ptr->setSize(w, h);

#define BUTTON(ptr, x, y, w, h, text) \
    ptr = std::make_unique<Button>(this, text); \
    ptr->setSize(w, h); \
    ptr->setPos(x, y);
    
#define LVGL_OBJ_FWD \
    struct _lv_obj_t; \
    typedef struct _lv_obj_t lv_obj_t
