#pragma once

#define LABEL(ptr, x, y, w, h, text) \
    ptr = std::make_unique<Label>(this, text); \
    ptr->setPos(x, y); \
    ptr->setSize(w, h);

#define BUTTON(ptr, x, y, w, h, text) \
    ptr = std::make_unique<Button>(this, text); \
    ptr->setSize(w, h); \
    ptr->setPos(x, y);
    