// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/Serializer.h"

#include "common/FileIO.h"

#include "core/primitives/ControlContext.h"

#include "core/primitives/AudioUnit.h"
#include "core/primitives/File.h"
#include "core/primitives/FileContainer.h"
#include "core/primitives/AudioRoute.h"
#include "core/primitives/MidiRoute.h"


#include "core/SettingsManager.h"
#include "core/Project.h"
#include "core/Timeline.h"
#include "core/FileTasks.h"
#include "core/FileWorker.h"

#include "core/Actions.h"

#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"

#include "logger.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>

namespace slr {



bool serialize(ControlContext &ctx) {
    //assume project has name(but it might not be unique)

    //if path.empty() than this is new project and never was saved before
    {
        std::filesystem::path check = SettingsManager::getProjectsPath();
        //check if default folder from settings exists
        if(!std::filesystem::exists(check.parent_path())) {
            std::filesystem::create_directory(check);
        }
    }

    if(ctx.projectView->path().empty()) {
        std::string tmppath = SettingsManager::getProjectsPath() + ctx.projectView->name();
        if(std::filesystem::exists(tmppath)) {
            //need to create another path
            std::string tmpname = ctx.projectView->name();
            tmpname += " ";
            tmpname += Common::FileIO::getDate();
            tmppath = SettingsManager::getProjectsPath() + tmpname;
            if(std::filesystem::exists(tmppath)) {
                LOG_ERROR("You need better saving system man...");
                return false;
            }

            ctx.projectView->name(tmpname);
        }

        ctx.projectView->path(tmppath);
    }

    std::filesystem::path projpath = ctx.projectView->path();
    if(!std::filesystem::exists(projpath)) {
        std::filesystem::create_directory(projpath);
    }

    //finally we can save something...
    /* Create json*/
    const slr::Timeline & timeline = ctx.project->timeline();
    nlohmann::ordered_json json;
    json["MAGIC"] = "SLRProject";
    json["Version"] = 0.01;
    json["Project Name"] = ctx.projectView->name();
    json["Timeline"]["BPM"] = timeline.bpm();
    json["Timeline"]["Bar Size"]["Numerator"] = timeline.getBarSize()._numerator;
    json["Timeline"]["Bar Size"]["Denominator"] = timeline.getBarSize()._denominator;
    json["Timeline"]["Loop Start"] = timeline.loopStartFrame();
    json["Timeline"]["Loop End"] = timeline.loopEndFrame();
    json["Timeline"]["Loop Enabled"] = timeline.looping();
    json["Timeline"]["Sample Rate"] = timeline.sampleRate();
    json["Timeline"]["Block Size"] = timeline.blockSize();

    const int unitCount = ctx.project->getUnitCount();
    json["Unit Count"] = unitCount;

    /* 
        Actually... emm...
        the AudioUnitView thing should be containing all necessary information 
        for saving project,
        so there is no need to call project by itself, but than...
        why i'm always doing extra checks on if uView != nullptr lol:D
        because changes in projectview isn't reliable yet - 
            there is no any synchronization so data may be corrupted?
    */
    std::vector<AudioUnitView*> uviews = ctx.projectView->unitList();
    for(AudioUnitView *uv : uviews) {
        nlohmann::ordered_json unit = uv->saveUnit();
        json["Units"].push_back(unit);
    }

    FileWorker *fw = ctx.fileWorker;
    const std::vector<File*> &files = fw->listFiles();
    json["Files Count"] = files.size();

    for(const File *f : files) {
        nlohmann::ordered_json filesub;

        auto saveTask = std::make_unique<Tasks::saveFile>();
        saveTask->fileId = f->id();
        ctx.fileWorker->addTask(std::move(saveTask));

        filesub["ID"] = f->id();
        // std::string fullpath = f->path();
        // if(fullpath.at(fullpath.size()) != '/') {
        //     fullpath += '/';
        // }

        // fullpath += f->name();

        filesub["Full Path"] = f->path();
        filesub["Offline"] = f->offline();
        json["Files"].push_back(filesub);
    }

    //clips(in general)
    ClipStorage & storage = ctx.project->clipStorage();
    const std::vector<ClipItem*> items = storage.items();
    json["Clips Count"] = items.size();
    for(const ClipItem *clip : items) {
        nlohmann::ordered_json subclip;

        subclip["ID"] = clip->id();
        subclip["Start"] = clip->startPosition();
        subclip["Length"] = clip->length();
        subclip["File Offset"] = clip->fileOffset();
        subclip["Is Muted"] = clip->isMuted();
        subclip["File ID"] = clip->_file->id();
        json["Clips"].push_back(subclip);
    }

    //routes
    const std::vector<AudioRoute> &audioRoutes = ctx.project->routes();
    json["Audio Routes Count"] = audioRoutes.size();
    for(const AudioRoute &r : audioRoutes) {
        nlohmann::ordered_json subroute;
        subroute["Source Type"] = r._sourceType == AudioRoute::Type::EXT ? "External" : "Internal";
        subroute["Source ID"] = r._sourceId;
        subroute["Target Type"] = r._targetType == AudioRoute::Type::EXT ? "External" : "Internal";
        subroute["Target ID"] = r._targetId;
        for(int i=0; i<32; ++i) {
            subroute["Channel Map"].push_back(r._channelMap[i]);
        }
        json["Audio Routes"].push_back(subroute);
    }

    const std::vector<MidiRoute> &midiRoutes = ctx.project->midiRoutes();
    json["Midi Routes Count"] = midiRoutes.size();
    for(const MidiRoute &r : midiRoutes) {
        nlohmann::ordered_json subroute;
        subroute["Source Type"] = r._sourceType == MidiRoute::Type::EXT ? "External" : "Internal";
        subroute["Source ID"] = r._sourceId;
        subroute["Source Channel"] = r._sourceChannel;
        subroute["Target Type"] = r._targetType == MidiRoute::Type::EXT ? "External" : "Internal";
        subroute["Target ID"] = r._targetId;
        subroute["Target Channel"] = r._targetChannel;
        json["Midi Routes"].push_back(subroute);
    }

    /* Dump To File */
    // std::string dump = json.dump();
    projpath += ("/" + ctx.projectView->name() + ".json");
    // projpath += ".json";
    std::ofstream f;
    f.open(projpath);
    if(!f.is_open()) {
        // LOG_ERROR("Failed to create config file at %s", path.c_str());
        return false;
    }

    f << json.dump(4) << std::endl;
    f.close();

    return true;
}

nlohmann::json Deserializer::loadJson(const std::string &path) {
    if(path.empty()) {
        LOG_ERROR("Path is empty");
        return nlohmann::json{};
    }
    if(!std::filesystem::exists(path)) {
        LOG_ERROR("File at path %s doesn't exists", path.c_str());
        return nlohmann::json{};
    }
    if(path.substr(path.size()-5, path.size()) != ".json") {
        LOG_ERROR("File extention is not json");
        return nlohmann::json{};
    }

    std::ifstream f;
    f.open(path);
    if(!f.is_open()) {
        LOG_ERROR("Failed to open file");
        return nlohmann::json{};
    }

    nlohmann::json ret = nlohmann::json::parse(f);
    f.close();

    return ret;
}

}