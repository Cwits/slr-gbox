// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/guiThread.h"

#include "lvgl.h"
#include "ui/display/MainWindow.h"
#include "ui/display/SplashScreen.h"
#include "ui/display/layoutSizes.h"
#include "ui/FakeGestures.h"

#include "logger.h"

#include <SDL2/SDL.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <fstream>
#include <iostream>
#include <functional>

#if defined(__aarch64__)
    #define SDL_HOR_RES 1920
    #define SDL_VER_RES 1080
#else
    #define SDL_HOR_RES UI::LayoutDef::TOTAL_WIDTH
    #define SDL_VER_RES UI::LayoutDef::TOTAL_HEIGHT
#endif

std::atomic<bool> _running;

lv_display_t *lvDisplay;
UI::SplashScreen * _splash;
lv_obj_t * _main_screen;

#if defined(__aarch64__)
GestLib::GestureRecognizer _recognizer;
#else
FakeGestures _fakeGestures;
#endif

std::ofstream _logFile;

//from uiControls.cpp
extern std::mutex g_async_mtx;
extern std::deque<std::unique_ptr<std::function<void()>>> g_async_queue;
extern std::atomic_bool g_dispatch_scheduled;


void my_log_cb(lv_log_level_t level, const char * buf) {
//   serial_send(buf, strlen(buf));
    _logFile << buf;
}


void print_sdl_event(const SDL_Event& event);

void initGui() {
    lv_init();

    // _logFile.open("./lvglLog.txt");
    // lv_log_register_print_cb(my_log_cb);

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
    // the name is set in lv_sdl_window.c:378
    lvDisplay = lv_sdl_window_create(SDL_HOR_RES, SDL_VER_RES);
    
    {
        //if output "opengles2" или "opengl" than accelerated 
        SDL_RendererInfo info;
        SDL_Renderer * renderer = static_cast<SDL_Renderer*>(lv_sdl_window_get_renderer(lvDisplay));
        SDL_GetRendererInfo(renderer, &info);
        printf("Renderer: %s\n", info.name);
    }
    // SDL_GL_SetSwapInterval(0);
    
    _main_screen = lv_screen_active();
    _splash = new UI::SplashScreen(nullptr);
    lv_screen_load(_splash->_host);
    lv_timer_handler();

#if defined(__aarch64__)
    if(!_recognizer.init()) {
        LOG_FATAL("Failed to init gesture recognizer");
    }

    _recognizer.windowSize(SDL_HOR_RES, SDL_VER_RES);
    if(!_recognizer.start()) {
        LOG_FATAL("Failed to start gesture recognizer");
    }

#else
    // if(SDL_CaptureMouse(SDL_TRUE) == -1) {
    //     LOG_ERROR("Failed to SDL_CaptureMouse");
    // }
    // SDL_SetRelativeMouseMode(SDL_TRUE);
#endif
}

void runGui() {
    _running = true;
    
    UI::MainWindow * main = new UI::MainWindow(_main_screen);
    lv_screen_load(_main_screen);

    const uint32_t delayTicksUs = LV_DEF_REFR_PERIOD * 1000; //to micros
    SDL_Event event;

    // uint32_t lastTick = SDL_GetTicks();
    std::chrono::time_point<std::chrono::steady_clock> lastTick = std::chrono::steady_clock::now();
    std::chrono::time_point<std::chrono::steady_clock> now = lastTick;
    std::chrono::time_point<std::chrono::steady_clock> elapsed = lastTick;
    
    while(_running) {
        now = std::chrono::steady_clock::now();
#if defined(__aarch64__)
        std::vector<GestLib::Gesture> gestures = _recognizer.fetchGestures();
        for(GestLib::Gesture &g : gestures) {
            UI::MainWindow::inst()->handleGesture(g);
        }
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                LOG_INFO("SDL quit");
                _running = false;
            }
        }
#else
        int counter = 0;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_KEYDOWN) {
                _fakeGestures.handleKeyboard(event.key);
                counter++;
                // LOG_INFO("Key pressed, %i", event.key.keysym.sym);
            } else if(event.type == SDL_KEYUP) {
                _fakeGestures.handleKeyboard(event.key);
                counter++;
                // LOG_INFO("Key released, %i", event.key.keysym.sym);
            } else if(event.type == SDL_MOUSEMOTION) {
                _fakeGestures.handleMouseMotion(event.motion);
                counter++;
                // LOG_INFO("mouse motion, x:%i, y:%i", event.motion.x, event.motion.y);
            } else if(event.type == SDL_MOUSEBUTTONDOWN) {
                _fakeGestures.handleMouseButton(event.button);
                counter++;
                // LOG_INFO("mouse button pressed, %i", event.button.button);
            } else if(event.type == SDL_MOUSEBUTTONUP) {
                _fakeGestures.handleMouseButton(event.button);
                counter++;
                // LOG_INFO("mouse button released, %i", event.button.button);
            } else if(event.type == SDL_QUIT) {
                //TODO: not always catching this event...
                LOG_INFO("Sdl quit");
                // _running = false;
                shutdown();
            }
            // print_sdl_event(event);
        }
#endif

        {
            while(true) {
                std::unique_ptr<std::function<void()>> task;
                {
                    std::lock_guard<std::mutex>lk(g_async_mtx);
                    if(g_async_queue.empty()) break;
                    task = std::move(g_async_queue.front()); g_async_queue.pop_front();
                }
                (*task)();
            }
        }

        uint32_t mills = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTick).count();
        
        lastTick = now;
        lv_tick_inc(mills); // Update the tick timer. Tick is new for LVGL 9
        uint32_t ret = lv_timer_handler(); // Update the UI-

        elapsed = std::chrono::steady_clock::now();
        mills = std::chrono::duration_cast<std::chrono::microseconds>(elapsed - now).count();
        // LOG_INFO("render frame time: %d", mills);
        if(delayTicksUs > mills) {
            // std::this_thread::sleep_for(std::chrono::microseconds(delayTicksUs - mills));
            std::this_thread::sleep_for(std::chrono::milliseconds(ret));
        }
    }

#if defined(__aarch64__)
    _recognizer.shutdown();
#endif

    delete main;
    delete _splash;

    lv_display_delete(lvDisplay);

    // lv_mem_monitor();???
    // lv_debug_dump_obj_list();???

    lv_sdl_quit();
    lv_deinit(); 

    // _logFile.close();
}

void shutdown() {
    _running = false;
}





// #include <SDL2/SDL.h>
// #include <iostream>

void print_sdl_event(const SDL_Event& event) {
    std::cout << "SDL_Event: ";
    switch (event.type) {
        case SDL_MOUSEMOTION:
            std::cout << "Mouse Motion - x: " << event.motion.x
                      << ", y: " << event.motion.y
                      << ", xrel: " << event.motion.xrel
                      << ", yrel: " << event.motion.yrel << std::endl;
            break;

        case SDL_MOUSEBUTTONDOWN:
            std::cout << "Mouse Button Down - button: " << (int)event.button.button
                      << ", x: " << event.button.x
                      << ", y: " << event.button.y << std::endl;
            break;

        case SDL_MOUSEBUTTONUP:
            std::cout << "Mouse Button Up - button: " << (int)event.button.button
                      << ", x: " << event.button.x
                      << ", y: " << event.button.y << std::endl;
            break;

        case SDL_MOUSEWHEEL:
            std::cout << "Mouse Wheel - x: " << event.wheel.x
                      << ", y: " << event.wheel.y << std::endl;
            break;

        case SDL_KEYDOWN:
            std::cout << "Key Down - scancode: " << event.key.keysym.scancode
                      << ", keycode: " << event.key.keysym.sym
                      << ", name: " << SDL_GetKeyName(event.key.keysym.sym) << std::endl;
            break;

        case SDL_KEYUP:
            std::cout << "Key Up - scancode: " << event.key.keysym.scancode
                      << ", keycode: " << event.key.keysym.sym
                      << ", name: " << SDL_GetKeyName(event.key.keysym.sym) << std::endl;
            break;
            
        case SDL_FINGERDOWN:
            std::cout << "Finger Down - fingerId: " << event.tfinger.fingerId
                      << ", x: " << event.tfinger.x
                      << ", y: " << event.tfinger.y << std::endl;
            break;

        case SDL_FINGERUP:
            std::cout << "Finger Up - fingerId: " << event.tfinger.fingerId
                      << ", x: " << event.tfinger.x
                      << ", y: " << event.tfinger.y << std::endl;
            break;
            
        case SDL_FINGERMOTION:
            std::cout << "Finger Motion - fingerId: " << event.tfinger.fingerId
                      << ", x: " << event.tfinger.x
                      << ", y: " << event.tfinger.y << std::endl;
            break;

        case SDL_WINDOWEVENT:
            switch (event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                    std::cout << "Window Resized - w: " << event.window.data1 << ", h: " << event.window.data2 << std::endl;
                    break;
                default:
                    std::cout << "Window Event - type: " << (int)event.window.event << std::endl;
                    break;
            }
            break;
        
        case SDL_QUIT:
            std::cout << "Quit Event" << std::endl;
            break;

        default:
            std::cout << "Unknown event type: " << event.type << std::endl;
            break;
    }
}