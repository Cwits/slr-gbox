// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <cstdio>
#include <iomanip>  //for getDateTime
#include <ctime>    //for getDateTime
#include <sstream>  //for getDateTime

namespace Common {

namespace FileIO {

bool fileExists(const std::string& path) {
    FILE* file = std::fopen(path.c_str(), "r");
    if (file) {
        std::fclose(file);
        return true;
    }
    return false;
}

bool deleteFile(const std::string& path) {
    return std::remove(path.c_str()) == 0;
}


//returns string with d-m-Y H-M-S
inline std::string getDateTime()
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::stringstream ss;
    ss << std::put_time(&tm, "%d-%m-%Y:%H-%M-%S");

    return ss.str();
}

inline std::string getDate()
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::stringstream ss;
    ss << std::put_time(&tm, "%d-%m-%Y");

    return ss.str();
}

//generate random string with specified length
//len should be > 1
inline std::string generateRandomName(int length)
{
    char letters[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };
    char randLetters[length] = {0};

    for(int i=0; i<length-1; ++i)
    {
        randLetters[i] = letters[rand()%26];
    }
    randLetters[length] = '\0';
    
    std::string str;
    str.append(randLetters);

    return str;
}

} //fileio

} //common
