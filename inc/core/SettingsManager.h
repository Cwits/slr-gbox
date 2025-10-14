// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include "defines.h"

/* 
    there should be many types of settings:
        original
        default

    if system is fresh than load original settings than, if user changes something he can save it by default, 
    so every time user start application the default would be used.

    so logic would be:
        init() {
            check if there is default.json
            if yes -> load it
            else -> load original
        }

        save() {
            if(dirty) -> save to default, 
            do not save anything to original
        }
*/
namespace slr {

namespace SettingsManager {

bool init(const std::string path = "");
bool save();
bool dirty();

/* Audio Driver */
const std::string & getAudioDriver();
void setAudioDriver(std::string & driver);

const int getSampleRate();
void setSampleRate(int samplerate);

const int getBlockSize();
void setBlockSize(int blockSize);

const frame_t getManualLatencyCompensation();
void setManualLatencyCompensation(frame_t compensation);


/* Project */
const float getBpm();
void setBpm(float bpm);

const int getPpqn();
void setPpqn(int ppqn);

const int getNumerator();
void setNumerator(int numerator);

const int getDenominator();
void setDenominator(int denominator);

/* System */
const std::string & getProjectsPath();
void setProjectsPath(std::string & path);

const bool getSaveTmpToProjectFolder();
void setSaveTmpToProjectFolder(bool save);

const std::string & getTmpRecordPath();
void setTmpRecordPath(std::string & path);

const std::string & getPluginPath();
void setPluginPath(std::string & path);


/* UI */
const int getDefaultPixelPerBar();
void setDefaultPixelPerBar(int pixel);

const slr::frame_t getFramesPerPixel();
void setFramesPerPixel(const slr::frame_t frames); 


/* Other */
std::string & getDefaultStorage();
void setDefaultStorage(std::string & path);

} //SettingsManager


} //slr