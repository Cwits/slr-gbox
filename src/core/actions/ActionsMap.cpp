// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/actions/ActionsMap.h"

#include "core/actions/Actions.h"
#include "core/actions/ActionExecutable.h"

// Audio Units
#include "core/actions/AudioUnit/SetParameter.h"
#include "core/actions/AudioUnit/LoadAsClip.h"
#include "core/actions/AudioUnit/RemoveClip.h"
#include "core/actions/AudioUnit/ToggleMidiThru.h"
#include "core/actions/AudioUnit/ToggleOmniHwInput.h"
#include "core/actions/AudioUnit/SetName.h"
#include "core/actions/AudioUnit/SetColor.h"

#include "core/actions/Project/CreateNewUnit.h"
#include "core/actions/Project/DeleteUnit.h"
#include "core/actions/Project/AddNewAudioRoute.h"
#include "core/actions/Project/AddNewMidiRoute.h"
#include "core/actions/Project/ModifyClipItem.h"
#include "core/actions/Project/SaveProject.h"
#include "core/actions/Project/LoadProject.h"
#include "core/actions/Project/UpdateRenderPlan.h"
#include "core/actions/Project/Undo.h"
#include "core/actions/Project/Redo.h"

#include "core/actions/Timeline/ChangeSignatureBpm.h"
#include "core/actions/Timeline/ChangeTimelineState.h"
#include "core/actions/Timeline/LoopPosition.h"
#include "core/actions/Timeline/ToggleLoop.h"
#include "core/actions/Timeline/UpdatePlayhead.h"

#include "core/actions/MidiController/ToggleMidiDevice.h"
#include "core/actions/MidiController/VirtualMidiKbdAction.h"

#include "core/actions/Metronome/ToggleMetronome.h"

#include "core/actions/StepSequencer/CreateNewSequence.h"
#include "core/actions/StepSequencer/ModifySequence.h"
#include "core/actions/StepSequencer/ModifySequenceLayer.h"
#include "core/actions/StepSequencer/ModifySequenceEvent.h"
#include "core/actions/StepSequencer/ModifySequenceTarget.h"

#include "core/actions/ModulationEngine/CreateNewModulation.h"
#include "core/actions/ModulationEngine/ModifyModulation.h"
#include "core/actions/ModulationEngine/ModifyModulationTarget.h"

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

    //Step Sequencer
    map[typeid(Actions::CreateNewSequence)] = &createCreateNewSequenceAction;
    map[typeid(Actions::ModifySequence)] = &createModifySequenceAction;
    map[typeid(Actions::ModifySequenceLayer)] = &createModifySequenceLayerAction;
    map[typeid(Actions::ModifySequenceEvent)] = &createModifySequenceEventAction;
    map[typeid(Actions::ModifySequenceTarget)] = &createModifySequenceTargetAction;

    //Modulation Engine
    map[typeid(Actions::CreateNewModulation)] = &createCreateNewModulationAction;
    map[typeid(Actions::ModifyModulation)] = &createModifyModulationAction;
    map[typeid(Actions::ModifyModulationTarget)] = &createModifyModulationTargetAction;
}

std::map<std::type_index, CreatorFn> & getActionMap() {
    static std::map<std::type_index, CreatorFn> actionMap;
    return actionMap;
}



}