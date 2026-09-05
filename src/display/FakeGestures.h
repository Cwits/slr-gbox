// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <SDL2/SDL.h>
#include "gestlib/GestLib.h"

struct FakeGestures {
    FakeGestures();
    ~FakeGestures();

    void handleKeyboard(SDL_KeyboardEvent & key);
    void handleMouseButton(SDL_MouseButtonEvent & button);
    void handleMouseMotion(SDL_MouseMotionEvent & motion);

    private:
    bool _once;
    bool _gestureOnGoing;
    int _startX;
    int _startY;
    GestLib::Gestures _gesture;

    void changeGesture(GestLib::Gestures gest);
    void makeTouchUp(int x, int y);
    void makeTouchDown(int x, int y);
    void makeTap(int x, int y);
    void makeHoldStart(int x, int y);
    void makeHoldMove(int x, int y);
    void makeHoldEnd(int x, int y);
    void makeDoubleTap(int x, int y);
    void makeDragStart(int x, int y);
    void makeDragMove(int x, int y);
    void makeDragEnd(int x, int y);
    void makeSwipeStart(int x, int y);
    void makeSwipeMove(int x, int y);
    void makeSwipeEnd(int x, int y);
    void makeDTSwipeStart(int x, int y);
    void makeDTSwipeMove(int x, int y);
    void makeDTSwipeEnd(int x, int y);
};