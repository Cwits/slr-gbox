// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display/FakeGestures.h"
#include "display/elements/RootWindow.h"

#include "common/logger.h"

FakeGestures::FakeGestures() {
    _gesture = GestLib::Gestures::Tap;
    _gestureOnGoing = false;
    _once = false;

    LOG_INFO("\n\
    Using fake gesture driver, press keyboard to change gesture: \n\
                Z - Tap\n\
                X - Hold\n\
                C - Drag\n\
                V - Swipe\n\
                B - Double Tap\n\
                N - Double Tap Swipe\n\
                I - Zoom In(on mouse click, single action)\n\
                O - Zoom Out(on mouse click, single action)\n\
    Than use Mouse Button to start and mouse move to move...\n");
}

FakeGestures::~FakeGestures() {

}

void FakeGestures::handleKeyboard(SDL_KeyboardEvent & key) {
    SDL_Scancode & code = key.keysym.scancode;
    std::string text;
    if(key.state == SDL_PRESSED) {
        if(code == SDL_SCANCODE_Z) {
            changeGesture(GestLib::Gestures::Tap);
            text = "Using fake gesture Tap";
            UI::RootWindow::inst()->floatingText(false, text);
        } else if(code == SDL_SCANCODE_X) {
            changeGesture(GestLib::Gestures::Hold);
            text = "Using fake gesture Hold";
            UI::RootWindow::inst()->floatingText(false, text);
        } else if(code == SDL_SCANCODE_C) {
            changeGesture(GestLib::Gestures::Drag);
            text = "Using fake gesture Drag";
            UI::RootWindow::inst()->floatingText(false, text);
        } else if(code == SDL_SCANCODE_V) {
            changeGesture(GestLib::Gestures::Swipe);
            text = "Using fake gesture Swipe";
            UI::RootWindow::inst()->floatingText(false, text);
        } else if(code == SDL_SCANCODE_B) {
            changeGesture(GestLib::Gestures::DoubleTap);
            text = "Using fake gesture Double Tap";
            UI::RootWindow::inst()->floatingText(false, text);
        } else if(code == SDL_SCANCODE_N) {
            changeGesture(GestLib::Gestures::DoubleTapSwipe);
            text = "Using fake gesture Double Tap Swipe";
            UI::RootWindow::inst()->floatingText(false, text);
        } else if(code == SDL_SCANCODE_R) {
            //reset...
        } else if(code == SDL_SCANCODE_I) {
            changeGesture(GestLib::Gestures::Zoom);
            _zoomIn = true;
            text = "Using fake gesture Zoom Plus";
            UI::RootWindow::inst()->floatingText(false, text);
        } else if(code == SDL_SCANCODE_O) {
            changeGesture(GestLib::Gestures::Zoom);
            _zoomIn = false;
            text = "Using fake gesture Zoom Minus";
            UI::RootWindow::inst()->floatingText(false, text);
        }
    }

    if(key.state == SDL_RELEASED) {
        _once = false;
    }
}

void FakeGestures::handleMouseButton(SDL_MouseButtonEvent & button) {
    // LOG_INFO("Clicked %i, gesture: %i", button.button, static_cast<int>(_gesture));
    if(button.button != 1) return; // == 3 for left click
    
    if(button.state == SDL_PRESSED) {
        // LOG_INFO("Pressed");
        switch(_gesture) {
            case(GestLib::Gestures::Tap): {
                makeTouchDown(button.x, button.y);
            } break;
            case(GestLib::Gestures::Hold): {
                makeTouchDown(button.x, button.y);
                makeHoldStart(button.x, button.y);
                _gestureOnGoing = true;
            } break;
            case(GestLib::Gestures::DoubleTap): {
                makeTouchDown(button.x, button.y);
            } break;
            case(GestLib::Gestures::Drag): {
                makeTouchDown(button.x, button.y);
                makeDragStart(button.x, button.y);
                _gestureOnGoing = true;
            } break;
            case(GestLib::Gestures::Swipe): {
                makeTouchDown(button.x, button.y);
                makeSwipeStart(button.x, button.y);
                _gestureOnGoing = true;
            } break;
            case(GestLib::Gestures::DoubleTapSwipe): {
                // makeDTSwipeStart(button.x, button.y);
                LOG_WARN("no such gesture yet");
                _gestureOnGoing = true;
            } break;
            case(GestLib::Gestures::Zoom): {
                makeTouchDown(button.x, button.y);
                if(_zoomIn) {
                    makeZoomIn(button.x, button.y);
                    // _gestureOnGoing = true;
                } else {
                    makeZoomOut(button.x, button.y);
                } 
            }
        }
    } else if(button.state == SDL_RELEASED) {
        // LOG_INFO("Released");
        switch(_gesture) { 
            case(GestLib::Gestures::Tap): {
                makeTap(button.x, button.y); 
                makeTouchUp(button.x, button.y);
            } break;
            case(GestLib::Gestures::Hold): {
                makeHoldEnd(button.x, button.y); 
                makeTouchUp(button.x, button.y);
                _gestureOnGoing = false;
            } break;
            case(GestLib::Gestures::DoubleTap): {
                makeDoubleTap(button.x, button.y); 
                makeTouchUp(button.x, button.y);
            } break;
            case(GestLib::Gestures::Drag):  {
                makeDragEnd(button.x, button.y); 
                makeTouchUp(button.x, button.y);
                _gestureOnGoing = false;
            } break;
            case(GestLib::Gestures::Swipe): {
                makeSwipeEnd(button.x, button.y); 
                makeTouchUp(button.x, button.y);
                _gestureOnGoing = false;
            } break;
            case(GestLib::Gestures::DoubleTapSwipe): {
                // makeDTSwipeEnd(button.x, button.y);
                LOG_WARN("no such gesture yet");
                _gestureOnGoing = false;
            } break;
            case(GestLib::Gestures::Zoom): {
                makeTouchUp(button.x, button.y);
            }
        }
    }
}

void FakeGestures::handleMouseMotion(SDL_MouseMotionEvent & motion) {
    if(!_gestureOnGoing) return;
    
    if(_gesture == GestLib::Gestures::Hold) {
        makeHoldMove(motion.x, motion.y);
    } else if(_gesture == GestLib::Gestures::Drag) {
        makeDragMove(motion.x, motion.y);
    } else if(_gesture == GestLib::Gestures::Swipe) {
        makeSwipeMove(motion.x, motion.y);
    } else if(_gesture == GestLib::Gestures::DoubleTapSwipe) {
        // makeDTSwipeMove(motion.x, motion.y);
    }
}

void FakeGestures::changeGesture(GestLib::Gestures gest) {
    if(!_once) {
        _once = true;
        _gesture = gest;
    }
}

void FakeGestures::makeTouchDown(int x, int y) {
    GestLib::Gesture down;
    down.type = GestLib::Gestures::TouchDown;
    down.touchDown.x = x;
    down.touchDown.y = y;
    UI::RootWindow::inst()->handleGesture(down);
}

void FakeGestures::makeTouchUp(int x, int y) {
    GestLib::Gesture up;
    up.type = GestLib::Gestures::TouchUp;
    up.touchUp.x = x;
    up.touchUp.y = y;
    UI::RootWindow::inst()->handleGesture(up);
}

void FakeGestures::makeTap(int x, int y) {
    GestLib::Gesture gesture;
    gesture.type = GestLib::Gestures::Tap;
    gesture.tap.x = x;
    gesture.tap.y = y;
    UI::RootWindow::inst()->handleGesture(gesture);
}

void FakeGestures::makeHoldStart(int x, int y) {
    GestLib::Gesture gesture;
    gesture.type = GestLib::Gestures::Hold;
    gesture.hold.state = GestLib::GestureState::Start;
    gesture.hold.x = x;
    gesture.hold.y = y;
    gesture.hold.dx = 0;
    gesture.hold.dy = 0;
    _startX = x;
    _startY = y;
    UI::RootWindow::inst()->handleGesture(gesture);
}

void FakeGestures::makeHoldMove(int x, int y) {
    GestLib::Gesture gesture;
    gesture.type = GestLib::Gestures::Hold;
    gesture.hold.state = GestLib::GestureState::Move;
    gesture.hold.x = x;
    gesture.hold.y = y;
    gesture.hold.dx = x - _startX;
    gesture.hold.dy = y - _startY;
    _startX = x;
    _startY = y;
    UI::RootWindow::inst()->handleGesture(gesture);
}

void FakeGestures::makeHoldEnd(int x, int y) {
    GestLib::Gesture gesture;
    gesture.type = GestLib::Gestures::Hold;
    gesture.hold.state = GestLib::GestureState::End;
    gesture.hold.x = x;
    gesture.hold.y = y;
    gesture.hold.dx = 0;
    gesture.hold.dy = 0;
    _startX = 0;
    _startY = 0;
    UI::RootWindow::inst()->handleGesture(gesture);
}

void FakeGestures::makeDoubleTap(int x, int y) {
    GestLib::Gesture gesture;
    gesture.type = GestLib::Gestures::DoubleTap;
    gesture.doubleTap.x = x;
    gesture.doubleTap.y = y;
    UI::RootWindow::inst()->handleGesture(gesture);
}

void FakeGestures::makeDragStart(int x, int y) {
    GestLib::Gesture drag;
    drag.type = GestLib::Gestures::Drag;
    drag.drag.state = GestLib::GestureState::Start;
    drag.drag.x = x;
    drag.drag.y = y;
    drag.drag.dx = 0;
    drag.drag.dy = 0;
    _startX = x;
    _startY = y;
    UI::RootWindow::inst()->handleGesture(drag);
}

void FakeGestures::makeDragMove(int x, int y) {
    GestLib::Gesture drag;
    drag.type = GestLib::Gestures::Drag;
    drag.drag.state = GestLib::GestureState::Move;
    drag.drag.x = x;
    drag.drag.y = y;
    drag.drag.dx = x - _startX;
    drag.drag.dy = y - _startY;
    _startX = x;
    _startY = y;
    UI::RootWindow::inst()->handleGesture(drag);
}

void FakeGestures::makeDragEnd(int x, int y) {
    GestLib::Gesture drag;
    drag.type = GestLib::Gestures::Drag;
    drag.drag.state = GestLib::GestureState::End;
    drag.drag.x = x;
    drag.drag.y = y;
    drag.drag.dx = 0;
    drag.drag.dy = 0;
    _startX = 0;
    _startY = 0;
    UI::RootWindow::inst()->handleGesture(drag);
}

void FakeGestures::makeSwipeStart(int x, int y) {
    GestLib::Gesture swipe;
    swipe.type = GestLib::Gestures::Swipe;
    swipe.swipe.state = GestLib::GestureState::Start;
    swipe.swipe.x = x;
    swipe.swipe.y = y;
    swipe.swipe.dx = 0;
    swipe.swipe.dy = 0;
    _startX = x;
    _startY = y;
    UI::RootWindow::inst()->handleGesture(swipe);    
}

void FakeGestures::makeSwipeMove(int x, int y) {
    GestLib::Gesture swipe;
    swipe.type = GestLib::Gestures::Swipe;
    swipe.swipe.state = GestLib::GestureState::Move;
    swipe.swipe.x = x;
    swipe.swipe.y = y;
    swipe.swipe.dx = x - _startX;
    swipe.swipe.dy = y - _startY;
    _startX = x;
    _startY = y;
    UI::RootWindow::inst()->handleGesture(swipe);    
}

void FakeGestures::makeSwipeEnd(int x, int y) {
    GestLib::Gesture swipe;
    swipe.type = GestLib::Gestures::Swipe;
    swipe.swipe.state = GestLib::GestureState::End;
    swipe.swipe.x = x;
    swipe.swipe.y = y;
    swipe.swipe.dx = _startX - x;
    swipe.swipe.dy = _startY - y;
    _startX = 0;
    _startY = 0;
    UI::RootWindow::inst()->handleGesture(swipe); 
}

void FakeGestures::makeDTSwipeStart(int x, int y) {

}

void FakeGestures::makeDTSwipeMove(int x, int y) {

}

void FakeGestures::makeDTSwipeEnd(int x, int y) {

}

void FakeGestures::makeZoomIn(int x, int y) {
    GestLib::Gesture zoom;
    zoom.type = GestLib::Gestures::Zoom;
    zoom.zoom.x1 = x;
    zoom.zoom.y1 = y;
    zoom.zoom.x2 = x;
    zoom.zoom.y2 = y;
    zoom.zoom.diff = 1.0f;
    UI::RootWindow::inst()->handleGesture(zoom); 
}
void FakeGestures::makeZoomOut(int x, int y) {
    GestLib::Gesture zoom;
    zoom.type = GestLib::Gestures::Zoom;
    zoom.zoom.x1 = x;
    zoom.zoom.y1 = y;
    zoom.zoom.x2 = x;
    zoom.zoom.y2 = y;
    zoom.zoom.diff = -1.0f;
    UI::RootWindow::inst()->handleGesture(zoom);
}
