// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "common/logger.h"

#include <string>
#include <cstdio>
#include <iomanip>  //for getDateTime
#include <ctime>    //for getDateTime
#include <sstream>  //for getDateTime
#include <algorithm>
#include <vector>

namespace Common {

namespace FileIO {

inline bool fileExists(const std::string& path) {
    FILE* file = std::fopen(path.c_str(), "r");
    if (file) {
        std::fclose(file);
        return true;
    }
    return false;
}

inline bool deleteFile(const std::string& path) {
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

enum class Extention { Audio, Midi, AudioPeak };
inline bool pathHasExtention(Extention e, const std::string & path) {
    bool ret = false;
    if(e == Extention::Audio) {
        if(path.substr(path.size()-4).compare(".wav") == 0) {
            ret = true;
        }
    } else if(e == Extention::Midi) {
        if(path.substr(path.size()-4).compare(".mid") == 0) {
            ret = true;
        }
    } else if(e == Extention::AudioPeak) {
        if(path.substr(path.size()-6).compare(".slrpk") == 0) {
            ret = true;
        }
    }

    return ret;
}

inline bool pathIsValid(const std::string &path, bool createFolders = false) {
    //TODO: create this func... lol :D
    //in case if target exists - should happen nothing, otherwise create folders
    return true;
}

const std::vector<std::string> KNOWN_EXTENTIONS = {
    ".wav", ".mp3", ".ogg", ".slrpk", ".json", ".mid"
};

inline bool changeExtentionTo(std::string &path, std::string newExtention) {
    bool success = false;
    //check for known extention
    std::string ext = path.substr(path.find_last_of('.')); 
    for(auto &s : KNOWN_EXTENTIONS) {
        if(ext == s) { success = true; break; }
    }

    if(!success) return false;

    path = path.substr(0, path.find_last_of('.'));
    path.append(newExtention);
    return true;
}

inline bool pathOrFileExists(const std::string &path) {
    if(path.find_last_of('.') != std::string::npos) {
        //checking file
        return fileExists(path);
    } else {
        //checking folder
        LOG_WARN("Folder checking not implemented");
        return true;
    }
}

} //fileio

} //common
