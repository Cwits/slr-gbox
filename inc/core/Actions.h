// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/primitives/ActionBase.h"
#include "core/primitives/AudioRoute.h"
#include "core/primitives/MidiRoute.h"
#include "common/Color.h"
#include "defines.h"

#include <variant>
#include <string>
#include <memory>
#include <functional>
#include <optional>
#include <nlohmann/json.hpp>

namespace slr {

struct File;
struct ClipItem;
struct MidiDevice;
struct MidiSubdevice;

namespace Actions {

/* Audio Unit Actions */
struct SetParameter : public ActionBase {
    SetParameter() {}
    SetParameter(const SetParameter &rhs) : 
        ActionBase(rhs), 
        targetId(rhs.targetId),
        parameterId(rhs.parameterId),
        value(rhs.value) {}

    std::type_index actionType() const override { return typeid(SetParameter); }
    
    ID targetId;
    ID parameterId;
    float value;
};

struct LoadAsClip : public ActionBase {
    LoadAsClip() {}
    LoadAsClip(const LoadAsClip &rhs) :
        ActionBase(rhs),
        data(rhs.data), targetId(rhs.targetId), startOffset(rhs.startOffset),
        fileOffset(rhs.fileOffset), makeUnique(rhs.makeUnique), length(rhs.length),
        isMuted(rhs.isMuted), clipForcedId(rhs.clipForcedId), fileForcedId(rhs.fileForcedId) {}

    std::type_index actionType() const override { return typeid(LoadAsClip); }

    std::variant<std::string, File*, ClipItem*> data;
    ID targetId;
    frame_t startOffset;

    std::optional<frame_t> fileOffset;
    std::optional<bool> makeUnique;
    std::optional<frame_t> length;
    std::optional<bool> isMuted;
    std::optional<ID> clipForcedId;
    std::optional<ID> fileForcedId;
};

struct RemoveClip : public ActionBase {
    RemoveClip() {}
    RemoveClip(const RemoveClip &rhs) :
        ActionBase(rhs),
        clipId(rhs.clipId),
        targetId(rhs.targetId) {}

    std::type_index actionType() const override { return typeid(RemoveClip); }

    ID clipId;
    ID targetId;
};

struct ToggleMidiThru : public ActionBase {
    ToggleMidiThru() {}
    ToggleMidiThru(const ToggleMidiThru &rhs) :
        ActionBase(rhs),
        targetId(rhs.targetId),
        newState(rhs.newState) {}

    std::type_index actionType() const override { return typeid(ToggleMidiThru); }

    ID targetId;
    bool newState;
};

struct ToggleOmniHwInput : public ActionBase {
    ToggleOmniHwInput() {}
    ToggleOmniHwInput(const ToggleOmniHwInput &rhs) :
        ActionBase(rhs),
        targetId(rhs.targetId),
        newState(rhs.newState) {}

    std::type_index actionType() const override { return typeid(ToggleOmniHwInput); }

    ID targetId;
    bool newState;
};

struct SetName : public ActionBase {
    SetName() {}
    SetName(const SetName &rhs) :
        ActionBase(rhs),
        targetId(rhs.targetId),
        newName(rhs.newName) {}

    std::type_index actionType() const override { return typeid(SetName); }

    ID targetId;
    std::string newName;
};

struct SetColor : public ActionBase {
    SetColor() {}
    SetColor(const SetColor &rhs) :
        ActionBase(rhs),
        targetId(rhs.targetId),
        color(rhs.color) {}
    
    std::type_index actionType() const override { return typeid(SetColor); }

    ID targetId;
    Color color;
};


/* Project Actions */
struct CreateNewUnit : public ActionBase {
    CreateNewUnit() {}
    CreateNewUnit(const CreateNewUnit &rhs) :
        ActionBase(rhs), name(rhs.name), forcedId(rhs.forcedId), restoredUnit(rhs.restoredUnit) {}

    std::type_index actionType() const override { return typeid(CreateNewUnit); }

    std::string name;
    
    std::optional<slr::ID> forcedId;
    std::optional<nlohmann::ordered_json> restoredUnit;
};


struct DeleteUnit : public ActionBase {
	DeleteUnit() {}
	DeleteUnit(const DeleteUnit &rhs) :
		ActionBase(rhs), targetId(rhs.targetId) {}
	
	std::type_index actionType() const override { return typeid(DeleteUnit); }
	
	ID targetId;
};

struct AddNewAudioRoute : public ActionBase {
	AddNewAudioRoute() {}
	AddNewAudioRoute(const AddNewAudioRoute &rhs) : 
		ActionBase(rhs), route(rhs.route), swapPlan(rhs.swapPlan) {}
		
	std::type_index actionType() const override { return typeid(AddNewAudioRoute); }
	
    AudioRoute route;
    bool swapPlan = true;
};

struct AddNewMidiRoute : public ActionBase {
	AddNewMidiRoute() {}
	AddNewMidiRoute(const AddNewMidiRoute &rhs) : 
		ActionBase(rhs), route(rhs.route), swapPlan(rhs.swapPlan) {}
		
	std::type_index actionType() const override { return typeid(AddNewMidiRoute); }
	
    MidiRoute route;
    bool swapPlan = true;
};

struct ModifyClipItem : public ActionBase {
	ModifyClipItem() {}
	ModifyClipItem(const ModifyClipItem &rhs) : 
		ActionBase(rhs), 
        clipId(rhs.clipId),
        startPosition(rhs.startPosition),
        length(rhs.length),
        fileStartOffset(rhs.fileStartOffset),
        muted(rhs.muted) {}
		
	std::type_index actionType() const override { return typeid(ModifyClipItem); }
	
    ID clipId;
    frame_t startPosition;
    frame_t length;
    frame_t fileStartOffset;
    bool muted;
};

struct SaveProject : public ActionBase { 
    SaveProject() {}
    SaveProject(const SaveProject &rhs) : 
        ActionBase(rhs) {}

    std::type_index actionType() const override { return typeid(SaveProject); }

};

struct LoadProject : public ActionBase {
    LoadProject() {}
    LoadProject(const LoadProject &rhs) :
        ActionBase(rhs),
        path(rhs.path) {}

    std::type_index actionType() const override { return typeid(LoadProject); }

    std::string path;
};

struct UpdateRenderPlan : public ActionBase {
    UpdateRenderPlan() {}
    UpdateRenderPlan(const UpdateRenderPlan &rhs) :
        ActionBase(rhs) {}

    std::type_index actionType() const override { return typeid(UpdateRenderPlan); }
};

struct Undo : public ActionBase {
    Undo() {}
    Undo(const Undo &rhs) : 
        ActionBase(rhs) {}
    
    std::type_index actionType() const override { return typeid(Undo); }
};

struct Redo : public ActionBase {
    Redo() {}
    Redo(const Redo &rhs) : 
        ActionBase(rhs) {}
    
    std::type_index actionType() const override { return typeid(Redo); }
};

/* Timeline */
struct ChangeSignatureBpm : public ActionBase {
    ChangeSignatureBpm() {}
    ChangeSignatureBpm(const ChangeSignatureBpm &rhs) :
        ActionBase(rhs), bpm(rhs.bpm), sig(rhs.sig) {}

    std::type_index actionType() const override { return typeid(ChangeSignatureBpm); }

    float bpm;
    slr::BarSize sig;
};

struct ChangeTimelineState : public ActionBase {
    ChangeTimelineState() {}
    ChangeTimelineState(const ChangeTimelineState &rhs) :
        ActionBase(rhs), state(rhs.state) {}

    std::type_index actionType() const override { return typeid(ChangeTimelineState); }

    TimelineState state;
};

struct LoopPosition : public ActionBase {
    LoopPosition() {}
    LoopPosition(const LoopPosition &rhs) :
        ActionBase(rhs), start(rhs.start), end(rhs.end) {}
    
    std::type_index actionType() const override { return typeid(LoopPosition); }

    frame_t start;
    frame_t end;
};

struct ToggleLoop : public ActionBase {
    ToggleLoop() {}
    ToggleLoop(const ToggleLoop &rhs) :
        ActionBase(rhs), newState(rhs.newState) {}
    
    std::type_index actionType() const override { return typeid(ToggleLoop); }

    bool newState;
};

struct UpdatePlayhead : public ActionBase {
    UpdatePlayhead() {}
    UpdatePlayhead(const UpdatePlayhead &rhs) :
        ActionBase(rhs) {}
    
    std::type_index actionType() const override { return typeid(UpdatePlayhead); }
};


/* Midi Controller */
struct ToggleMidiDevice : public ActionBase {
    ToggleMidiDevice() {}
    ToggleMidiDevice(const ToggleMidiDevice &rhs) :
        ActionBase(rhs),
        device(rhs.device),
        subdev(rhs.subdev),
        port(rhs.port),
        newState(rhs.newState),
        completed(rhs.completed) {}
    
    std::type_index actionType() const override { return typeid(ToggleMidiDevice); }


    MidiDevice * device;
    MidiSubdevice *subdev;
    DevicePort port;
    bool newState;
    std::function<void(int)> completed;
};

//Virtual Midi Keyboard Trigger
struct VMKTrigger : public ActionBase {
    VMKTrigger() {}
    VMKTrigger(const VMKTrigger &rhs) :
        ActionBase(rhs),
        note(rhs.note),
        velocity(rhs.velocity),
        channel(rhs.channel),
        isPressed(rhs.isPressed) {}

    std::type_index actionType() const override { return typeid(VMKTrigger); }

    int note;
    int velocity;
    int channel;
    bool isPressed;
};

/* Metronome */
struct ToggleMetronome : public ActionBase { 
    ToggleMetronome() {}
    ToggleMetronome(const ToggleMetronome &rhs) :
        ActionBase(rhs) {}

        std::type_index actionType() const override { return typeid(ToggleMetronome); }
};

} //namespace Actions

void EmitAction(std::unique_ptr<ActionBase> action);

} //namespace slr
