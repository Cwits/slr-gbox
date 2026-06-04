// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/ActionsMap.h"

#include "core/Actions.h"
#include "core/primitives/ActionExecutable.h"

// Audio Units
#include "core/Actions/AudioUnit/SetParameter.h"
#include "core/Actions/AudioUnit/LoadAsClip.h"
#include "core/Actions/AudioUnit/RemoveClip.h"
#include "core/Actions/AudioUnit/ToggleMidiThru.h"
#include "core/Actions/AudioUnit/ToggleOmniHwInput.h"
#include "core/Actions/AudioUnit/SetName.h"
#include "core/Actions/AudioUnit/SetColor.h"

#include "core/Actions/Project/CreateNewUnit.h"
#include "core/Actions/Project/DeleteUnit.h"
#include "core/Actions/Project/AddNewAudioRoute.h"
#include "core/Actions/Project/AddNewMidiRoute.h"
#include "core/Actions/Project/ModifyClipItem.h"
#include "core/Actions/Project/SaveProject.h"
#include "core/Actions/Project/LoadProject.h"
#include "core/Actions/Project/UpdateRenderPlan.h"
#include "core/Actions/Project/Undo.h"
#include "core/Actions/Project/Redo.h"

#include "core/Actions/Timeline/ChangeSignatureBpm.h"
#include "core/Actions/Timeline/ChangeTimelineState.h"
#include "core/Actions/Timeline/LoopPosition.h"
#include "core/Actions/Timeline/ToggleLoop.h"
#include "core/Actions/Timeline/UpdatePlayhead.h"

#include "core/Actions/MidiController/ToggleMidiDevice.h"
#include "core/Actions/MidiController/VirtualMidiKbdAction.h"

#include "core/Actions/Metronome/ToggleMetronome.h"

namespace slr {

void registerDefaultActions(std::map<std::type_index, CreatorFn> & map) {
    // std::map<std::type_index, CreatorFn> & map = getActionMap();

    //AudioUnit
    map[typeid(Actions::SetParameter)] =  &createSetParameterAction;
    map[typeid(Actions::LoadAsClip)] =  &createLoadAsClipAction;
    map[typeid(Actions::RemoveClip)] =  &createRemoveClipAction;
    map[typeid(Actions::ToggleMidiThru)] =  &createToggleMidiThruAction;
    map[typeid(Actions::ToggleOmniHwInput)] =  &createToggleOmniHwAction;
    map[typeid(Actions::SetName)] = &createSetNameAction;
    map[typeid(Actions::SetColor)] = &createSetColorAction;

    //Project
    map[typeid(Actions::CreateNewUnit)] = &createCreateNewUnitAction;
    map[typeid(Actions::DeleteUnit)] = &createDeleteUnitAction;
    map[typeid(Actions::AddNewAudioRoute)] = &createAddNewRouteAction;
    map[typeid(Actions::AddNewMidiRoute)] = &createAddNewMidiRouteAction;
    map[typeid(Actions::ModifyClipItem)] = &createModifyClipItemAction;
    map[typeid(Actions::SaveProject)] = &createSaveProjectAction;
    map[typeid(Actions::LoadProject)] = &createLoadProjectAction;
    map[typeid(Actions::UpdateRenderPlan)] = &createUpdateRenderPlanAction;
    map[typeid(Actions::Undo)] = &createUndoAction;
    map[typeid(Actions::Redo)] = &createRedoAction;   

    //Timeline
    map[typeid(Actions::ChangeSignatureBpm)] = &createChangeSignatureBpmAction;
    map[typeid(Actions::ChangeTimelineState)] = &createChangeTimelineStateAction;
    map[typeid(Actions::LoopPosition)] = &createLoopPositionAction;
    map[typeid(Actions::ToggleLoop)] = &createToggleLoopAction;
    map[typeid(Actions::UpdatePlayhead)] = &createUpdatePlayheadAction;

    //MidiController
    map[typeid(Actions::ToggleMidiDevice)] = &createToggleMidiDeviceAction;
    map[typeid(Actions::VMKTrigger)] = &createVMKTriggerAction;

    //Metronome
    map[typeid(Actions::ToggleMetronome)] = &createToggleMetronomeAction;
}

std::map<std::type_index, CreatorFn> & getActionMap() {
    static std::map<std::type_index, CreatorFn> actionMap;
    return actionMap;
}



}