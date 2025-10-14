// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "loguru.hpp"

/*
//g++ main.cpp -O3 -DNDEBUG -o app_release
cmake -DCMAKE_BUILD_TYPE=Release ./..
cmake -DCMAKE_BUILD_TYPE=Debug ./..
*/

#ifdef NDEBUG
    // Release: отключаем DEBUG и INFO
    #define LOG_DEBUG(...)   ((void)0)
    #define LOG_INFO(...)    ((void)0)
    #define LOG_WARN(...)    LOG_F(WARNING, __VA_ARGS__)
    #define LOG_ERROR(...)   LOG_F(ERROR, __VA_ARGS__)
    #define LOG_FATAL(...)   LOG_F(FATAL, __VA_ARGS__)
#else
    // Debug: всё включено
    #define LOG_DEBUG(...)   LOG_F(DEBUG, __VA_ARGS__)
    #define LOG_INFO(...)    LOG_F(INFO, __VA_ARGS__)
    #define LOG_WARN(...)    LOG_F(WARNING, __VA_ARGS__)
    #define LOG_ERROR(...)   LOG_F(ERROR, __VA_ARGS__)
    #define LOG_FATAL(...)   LOG_F(FATAL, __VA_ARGS__)
#endif
