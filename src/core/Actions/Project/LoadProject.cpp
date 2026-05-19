// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Actions/Project/LoadProject.h"
#include "core/primitives/ActionBase.h"

#include "core/ControlEngine.h"
#include "core/Serializer.h"
#include "core/UnitManager.h"
#include "core/Project.h"

#include "snapshots/ProjectView.h"
#include "snapshots/AudioUnitView.h"
#include "common/Color.h"

#include "logger.h"

#include <cassert>

namespace slr {

LoadProjectAction::LoadProjectAction(const ActionBase *base) :
    _action( *(static_cast<const Actions::LoadProject*>(base)) ),
    _foundUnits(0),
    _unitsNotFound(0)
{
}

LoadProjectAction::~LoadProjectAction() {

}

void LoadProjectAction::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    switch(_step) {
        case(1): {
            ControlEngine::prepareForProjectLoading();
            
            //acquire json
            _dataToLoad = Deserializer::loadJson(_action.path);
            
            if(_dataToLoad.at("MAGIC") != "SLRProject") {
                LOG_ERROR("MAGIC not working");
                abortAction();
                return;
            }

            std::size_t pos = _action.path.find_last_of('/');
            std::string name = _action.path.substr(pos+1);
            std::string path = _action.path.substr(0, pos);
            ctx.projectView->path(path);
            ctx.projectView->name(name.substr(0, name.find_last_of('.')));
            //restore timeline settings

            //create all units
            for(const auto& sub : _dataToLoad["Units"]) {
                ID idToForce = sub["ID"].get<ID>();
                std::string type = sub["Type"].get<std::string>();

                if(type.empty()) {
                    LOG_ERROR("Type is empty");
                    continue;
                }

                if(idToForce == 0) {
                    LOG_ERROR("ID can't be 0");
                    continue;
                }

                std::size_t found = type.find_first_of(':');
                if(type.substr(0, found) == "internal") {
                    //TODO: need somehow check that unit of that type available
                    std::string unitName = type.substr(found+1);
                    
                    if(!UnitManagerFactory::findUnit(unitName)) {
                        LOG_ERROR("Unit of type %s not registred in system, ignoring", unitName.c_str());
                        _unitsNotFound++;
                        continue;
                    }

                    auto act = std::make_unique<Actions::CreateNewUnit>();
                    act->name = unitName;
                    act->forcedId = idToForce;
                    EmitAction(std::move(act));
                    _foundUnits++;
                } else {
                    //external
                    LOG_WARN("external things not supported yet");
                    //might be vst/lv2 or whatever
                    //expected format:
                    // "external:vst3:pluginid"
                    // "external:lv2:uri"
                }
            }

            setState(ActionState::Waiting);
        } break;
        case(2): {
            //load clips, parameters, color, name, etc...
            
            for(const auto& sub : _dataToLoad["Units"]) {
                ID id = sub["ID"].get<ID>();
                std::string type = sub["Type"].get<std::string>();

                AudioUnitView *uView = ctx.projectView->getUnitById(id);
                if(!uView) {
                    LOG_ERROR("Failed to find view for ID %u", id);
                    continue;
                }

                uView->loadUnit(sub);

            }
            
            auto findClip = [](nlohmann::json &data, const ID clipId) -> const nlohmann::json * {
                if(data["Clips Count"].get<std::size_t>() == 0) return nullptr;
                for(const auto &cl : data["Clips"]) {
                    ID id = cl["ID"].get<ID>();
                    if(id != clipId) continue;

                    return &cl;
                }

                return nullptr;
            };

            auto findFile = [](nlohmann::json &data, const ID fileId) -> const nlohmann::json * {
                if(data["Files Count"].get<std::size_t>() == 0) return nullptr;
                for(const auto &fl : data["Files"]) {
                    ID id = fl["ID"].get<ID>();
                    if(id != fileId) continue;

                    return &fl;
                }

                return nullptr;
            };
            
            //create clips(+load files)
            for(const auto &sub : _dataToLoad["Units"]) {
                ID subid = sub["ID"].get<ID>();
                std::size_t clipCount = sub["Clips"]["Count"].get<std::size_t>();
                if(clipCount == 0) continue;

                std::vector<ID> clipsIds;
                clipsIds.reserve(4);
                for(const auto &clid : sub["Clips"]["Clips ID"])
                    clipsIds.push_back(clid.get<ID>());

                //now find files with such ids and load them
                LOG_INFO("For Unit %u loading:", subid);
                for(const ID clipId : clipsIds) {
                    const auto *cld = findClip(_dataToLoad, clipId);
                    if(!cld) {
                        LOG_ERROR("Failed to find clip, probably project file is corrupted, abort loading project");
                        abortAction();
                        return;
                    }

                    const nlohmann::json & clipData = *cld;
                    const ID fileId = clipData["File ID"].get<ID>();
                    const auto *fld = findFile(_dataToLoad, fileId);
                    if(!fld) {
                        LOG_ERROR("Failed to find file, probably project file is curropted, abort loading project");
                        abortAction();
                        return;
                    }
                    const nlohmann::json &fileData = *fld;

                    auto act = std::make_unique<Actions::LoadAsClip>();
                    act->targetId = sub["ID"].get<ID>();
                    act->data = fileData["Full Path"].get<std::string>();
                    act->makeUnique = false;
                    act->startOffset = clipData["Start"].get<frame_t>();
                    act->length = clipData["Length"].get<frame_t>();
                    act->fileOffset = clipData["File Offset"].get<frame_t>();
                    act->isMuted = clipData["Is Muted"].get<bool>();
                    act->clipForcedId = clipId;
                    act->fileForcedId = fileId;

                    /*
                    // LOG_INFO("file %s with: \n \
                    //             start offset: %lu \n \
                    //             length: %lu \n \
                    //             file offset: %lu \n \
                    //             is muted: %s \n \
                    //             clip forced id: %u \n \
                    //             file forced id: %u",
                    //     std::get<std::string>(act->data).c_str(),
                    //     act->startOffset,
                    //     act->length.value(),
                    //     act->fileOffset.value(),
                    //     (act->isMuted.value() ? "true" : "false"),
                    //     act->clipForcedId.value(),
                    //     act->fileForcedId.value()
                    // ); */
                    EmitAction(std::move(act));
                }
            }

            std::size_t routeCount = _dataToLoad["Audio Routes Count"];
            std::size_t counter = 0;
            //create routes
            for(const auto &sub : _dataToLoad["Audio Routes"]) {
                AudioRoute r;
                r._sourceId = sub["Source ID"].get<ID>();
                r._sourceType = sub["Source Type"].get<std::string>().compare("External") == 0 ? AudioRoute::Type::EXT : AudioRoute::Type::INT;
                r._targetId = sub["Target ID"].get<ID>();
                r._targetType = sub["Target Type"].get<std::string>().compare("External") == 0 ? AudioRoute::Type::EXT : AudioRoute::Type::INT;
                for(int i=0; i<32; ++i) {
                    r._channelMap[i] = sub["Channel Map"].at(i).get<int8_t>();
                }

                auto act = std::make_unique<Actions::AddNewAudioRoute>();
                act->route = r;
                act->swapPlan = false;
                EmitAction(std::move(act));
                counter++;
            }

            if(counter != routeCount) {
                LOG_WARN("Wrong created routes count");
            }

            routeCount = _dataToLoad["Midi Routes Count"];
            counter = 0;
            for(const auto &sub : _dataToLoad["Midi Routes"]) {
                MidiRoute r;
                r._sourceId = sub["Source ID"].get<ID>();
                r._sourceType = sub["Source Type"].get<std::string>().compare("External") == 0 ? MidiRoute::Type::EXT : MidiRoute::Type::INT;
                r._sourceChannel = sub["Source Channel"].get<int8_t>();
                r._targetId = sub["Target ID"].get<ID>();
                r._targetType = sub["Target Type"].get<std::string>().compare("External") == 0 ? MidiRoute::Type::EXT : MidiRoute::Type::INT;
                r._targetChannel = sub["Target Channel"].get<int8_t>();
                
                auto act = std::make_unique<Actions::AddNewMidiRoute>();
                act->route = r;
                act->swapPlan = false;
                EmitAction(std::move(act));
                counter++;
            }

            if(counter != routeCount) {
                LOG_WARN("Wrong created routes count");
            }
            
            setState(ActionState::Waiting);
        } break;
        case(3): {
            //have to wait till all routes created than need to update plan...
            auto act = std::make_unique<Actions::UpdateRenderPlan>();
            EmitAction(std::move(act));

            markDelete();
            setState(ActionState::Finished);
        } break;
        default: assert(false && "Unreachable"); break;
    }
}

void LoadProjectAction::checkWaitingCondition(ControlContext &ctx) {
	assert(getState() == ActionState::Waiting);

    switch(_step) {
        case(1): {
            std::size_t count = ctx.project->getUnitCount();
            if(count == static_cast<std::size_t>(_foundUnits)) {
                _step = 2;
                setState(ActionState::Executing);
            }
        } break;
        case(2): {
            std::size_t arouteCount = _dataToLoad["Audio Routes Count"];
            std::size_t mrouteCount = _dataToLoad["Midi Routes Count"];
            
            std::size_t aroutes = ctx.project->routes().size();
            std::size_t mroutes = ctx.project->midiRoutes().size();

            if(aroutes+mroutes == arouteCount+mrouteCount) {
                _step = 3;
                setState(ActionState::Executing);
            }

        } break;
        default: assert(false && "Unreachable"); break;
    }
}

std::unique_ptr<ActionExecutable> createLoadProjectAction(const ActionBase *base) {
    return std::make_unique<LoadProjectAction>(base);
}

}
