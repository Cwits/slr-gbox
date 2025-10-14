// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/SettingsManager.h"
#include "utility.h"
#include "logger.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>

bool _dirty = false;

/* settings varialbes */
/* Audio Driver */
std::string _audioDriver;
int _samplerate;
int _blockSize;
slr::frame_t _manualLatencyCompensation;

/* Project */
float _bpm;
int _ppqn;
int _numerator;
int _denominator;

/* System */
std::string _projectsPath;
bool _saveTmpToProjectFolder;
std::string _tmpRecordPath;
std::string _pluginPath;

/* UI */
int _defaultPixelPerBar;
slr::frame_t _defaultFramesPerPixel;

/* Other */
std::string _defaultStoragePath;

//compiling on pi change this path, as well paths in config folder
const std::string _pathPostfix = "/slr/config/";
std::string _configPath;

namespace slr {
    
namespace SettingsManager {
    // using json = nlohmann::json;

    // std::ifstream f("./../config/default.json");
    // json data = json::parse(f);
    // std::string test = "Jack Driver";
    // if(data.at("Audio Driver") == test) {
    //     LOG_F(INFO, "Happy");
    // }
    
    // if(data.at("Sample Rate") == 44100) {
    //     LOG_F(INFO, "Happy twice");
    // }
void buildConfig(const std::string &path);

std::string getHomeDir() {
    if (const char* home = std::getenv("HOME")) {
        return home;
    }
    struct passwd* pw = getpwuid(getuid());
    return pw ? pw->pw_dir : "";
}

bool init(const std::string path) {
    std::ifstream f;

    _configPath = getHomeDir();
    _configPath.append(_pathPostfix);

    nlohmann::json jsettings;
    std::string lpath;
    
    if(!path.empty()) {
        lpath = path;
    } else {
        lpath = _configPath;
    }
    
    lpath.append("default_config.json");
    
    if(file_exists(lpath)) {
        //load default
        f.open(lpath);
    } else {
        //build config
        buildConfig(lpath);
        f.open(lpath);
    }

    if(f.is_open()) {
        jsettings = nlohmann::json::parse(f);
        f.close();
    } else {
        LOG_FATAL("Failed to parse settings file");
        return false;
    }


    //parse and set values
    {
        //TODO: wrap around in try catch
        _audioDriver = jsettings.at("Audio Driver");
        _samplerate = jsettings.at("Sample Rate");
        _blockSize = jsettings.at("Block Size");
        _manualLatencyCompensation = jsettings.at("Latency Compensation");

        _bpm = jsettings.at("BPM");
        _ppqn = jsettings.at("PPQN");
        _numerator = jsettings.at("Numerator");
        _denominator = jsettings.at("Denominator");

        _projectsPath = jsettings.at("Projects Path");
        _saveTmpToProjectFolder = jsettings.at("Save Tmp to Project");
        _tmpRecordPath = jsettings.at("Temporary Recording Path");
        _pluginPath = jsettings.at("Plugins Path");

        _defaultPixelPerBar = jsettings.at("Default Pixel Per Bar");
        _defaultFramesPerPixel = jsettings.at("Frames Per Pixel");
    
        _defaultStoragePath = jsettings.at("Default Storage");
    }

    return true;
}

void buildConfig(const std::string &path) {
    std::string homedir = getHomeDir();
    std::string projects_path = homedir;
    projects_path.append("/music/projects/");
    std::string default_storage = homedir;
    default_storage.append("/music/");
    std::string plugins_path = homedir;
    plugins_path.append("/plugins/");
    std::string record_path = homedir;
    record_path.append("/music/recordings/");

    nlohmann::ordered_json settings = {
        {"Audio Driver", "Jack Driver"},
        {"Sample Rate", 44100},
        {"Block Size", 64},
        {"Latency Compensation", 40},
        {"BPM", 120.00},
        {"PPQN", 960},
        {"Numerator", 4},
        {"Denominator", 4},
        {"Projects Path", projects_path},
        {"Save Tmp to Project", true},
        {"Default Storage", default_storage},
        // {"Temporary Recording Path", std::filesystem::temp_directory_path()},
        {"Temporary Recording Path", record_path},
        {"Plugins Path", plugins_path},
        {"Default Pixel Per Bar", 3000},
        {"Frames Per Pixel", 588}
    };
    std::string dump = settings.dump();
    std::ofstream f;
    f.open(path);
    if(!f.is_open()) {
        LOG_ERROR("Failed to create config file at %s", path.c_str());
        return;
    }

    f << settings.dump(4) << std::endl;
    f.close();
}

bool save() {
    if(_dirty) {
        //save
        LOG_WARN("No settings saving implemented");
        return true;
    }

    return false;
}

bool dirty() {
    return _dirty;
}

/* Audio Driver */
const std::string & getAudioDriver() {
    return _audioDriver;
}

void setAudioDriver(std::string & driver) {
    _audioDriver = driver;
    _dirty = true;
}

const int getSampleRate() {
    return _samplerate;
}

void setSampleRate(int samplerate) {
    _samplerate = samplerate;
    _dirty = true;
}

const int getBlockSize() {
    return _blockSize;
}

void setBlockSize(int blockSize) {
    _blockSize = blockSize;
    _dirty = true;
}

const frame_t getManualLatencyCompensation() {
    return _manualLatencyCompensation;
}

void setManualLatencyCompensation(frame_t compensation) {
    _manualLatencyCompensation = compensation;
    _dirty = true;
}


/* Project */
const float getBpm() {
    return _bpm;
}

void setBpm(float bpm) {
    _bpm = bpm;
    _dirty = true;
}

const int getPpqn() {
    return _ppqn;
}

void setPpqn(int ppqn) {
    _ppqn = ppqn;
    _dirty = true;
}

const int getNumerator() {
    return _numerator;
}

void setNumerator(int numerator) {
    _numerator = numerator;
    _dirty = true;
}

const int getDenominator() {
    return _denominator;
}

void setDenominator(int denominator) {
    _denominator = denominator;
    _dirty = true;
}


/* System */
//default path where all projects would be stored - create separate folder for each project
const std::string & getProjectsPath() {
    return _projectsPath;
}

void setProjectsPath(std::string & path) {
    _projectsPath = path;
    _dirty = true;
}

//if set true than temporary files will be saved to current project folder
const bool getSaveTmpToProjectFolder() {
    return _saveTmpToProjectFolder;
}

void setSaveTmpToProjectFolder(bool save) {
    _saveTmpToProjectFolder = save;
    _dirty = true;
}

const std::string & getTmpRecordPath() {
    return _tmpRecordPath;
}

void setTmpRecordPath(std::string & path) {
    _tmpRecordPath = path;
    _dirty = true;
}

const std::string & getPluginPath() {
    return _pluginPath;
}

void setPluginPath(const std::string & path) {
    _pluginPath = path;
    _dirty = true;
}


/* UI */

//for zoom=1.0f, 120.00bpm, 4/4
const int getDefaultPixelPerBar() {
    return _defaultPixelPerBar;
}

void setDefaultPixelPerBar(const int pixel) {
    _defaultPixelPerBar = pixel;
    _dirty = true;
}

const slr::frame_t getFramesPerPixel() {
    return _defaultFramesPerPixel;
}

void setFramesPerPixel(const slr::frame_t frames) {
    _defaultFramesPerPixel = frames;
    _dirty = true;
}


/* Other */
std::string & getDefaultStorage() {
    return _defaultStoragePath;
}

void setDefaultStorage(std::string & path) {
    _defaultStoragePath = path;
    _dirty = true;
}

} //SettingsManager

} //slr