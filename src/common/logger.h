// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "loguru.hpp"

/*
//g++ main.cpp -O3 -DNDEBUG -o app_release
cmake -DCMAKE_BUILD_TYPE=Release ./..
cmake -DCMAKE_BUILD_TYPE=Debug ./..

// Source - https://stackoverflow.com/a/1961222
// Posted by epatel, modified by community. See post 'Timeline' for change history
// Retrieved 2026-09-19, License - CC BY-SA 2.5

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      // Black 
#define RED     "\033[31m"      // Red 
#define GREEN   "\033[32m"      // Green 
#define YELLOW  "\033[33m"      // Yellow 
#define BLUE    "\033[34m"      // Blue 
#define MAGENTA "\033[35m"      // Magenta 
#define CYAN    "\033[36m"      // Cyan 
#define WHITE   "\033[37m"      // White 
#define BOLDBLACK   "\033[1m\033[30m"      //Bold Black 
#define BOLDRED     "\033[1m\033[31m"      //Bold Red 
#define BOLDGREEN   "\033[1m\033[32m"      //Bold Green 
#define BOLDYELLOW  "\033[1m\033[33m"      //Bold Yellow 
#define BOLDBLUE    "\033[1m\033[34m"      //Bold Blue 
#define BOLDMAGENTA "\033[1m\033[35m"      //Bold Magenta 
#define BOLDCYAN    "\033[1m\033[36m"      //Bold Cyan 
#define BOLDWHITE   "\033[1m\033[37m"      //Bold White 

*/

#ifdef NDEBUG
    #define LOG_DEBUG(...)   ((void)0)
    #define LOG_INFO(...)    ((void)0)
    #define LOG_SUCCESS(fmt, ...) ((void)0)
    #define LOG_FAIL(fmt, ...) ((void)0)
    #define LOG_WARN(...)    LOG_F(WARNING, __VA_ARGS__)
    #define LOG_ERROR(...)   LOG_F(ERROR, __VA_ARGS__)
    #define LOG_FATAL(...)   LOG_F(FATAL, __VA_ARGS__)
#else
    #define LOG_DEBUG(...)   LOG_F(DEBUG, __VA_ARGS__)
    #define LOG_INFO(...)    LOG_F(INFO, __VA_ARGS__)
    #define LOG_SUCCESS(fmt, ...) LOG_F(INFO, "\033[32m" fmt "\033[0m", ##__VA_ARGS__)
    #define LOG_FAIL(fmt, ...) LOG_F(INFO, "\033[31m" fmt "\033[0m", ##__VA_ARGS__)
    #define LOG_WARN(...)    LOG_F(WARNING, __VA_ARGS__)
    #define LOG_ERROR(...)   LOG_F(ERROR, __VA_ARGS__)
    #define LOG_FATAL(...)   LOG_F(FATAL, __VA_ARGS__)
#endif
