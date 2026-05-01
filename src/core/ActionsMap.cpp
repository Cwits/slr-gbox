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

#include "core/Actions/Project/CreateNewUnit.h"
#include "core/Actions/Project/DeleteUnit.h"
#include "core/Actions/Project/AddNewAudioRoute.h"
#include "core/Actions/Project/AddNewMidiRoute.h"
#include "core/Actions/Project/ModifyClipItem.h"

#include "core/Actions/Timeline/ChangeSignatureBpm.h"
#include "core/Actions/Timeline/ChangeTimelineState.h"
#include "core/Actions/Timeline/LoopPosition.h"
#include "core/Actions/Timeline/ToggleLoop.h"
#include "core/Actions/Timeline/UpdatePlayhead.h"

#include "core/Actions/MidiController/ToggleMidiDevice.h"
#include "core/Actions/MidiController/VirtualMidiKbdAction.h"

#include "core/Actions/Metronome/ToggleMetronome.h"

namespace slr {

const std::map<std::type_index, CreatorFn> _actionMap = {
    //AudioUnit
    { typeid(Actions::SetParameter), &createSetParameterAction },
    { typeid(Actions::LoadAsClip), &createLoadAsClipAction },
    { typeid(Actions::RemoveClip), &createRemoveClipAction },
    { typeid(Actions::ToggleMidiThru), &createToggleMidiThruAction },
    { typeid(Actions::ToggleOmniHwInput), &createToggleOmniHwAction },

    //Project
    { typeid(Actions::CreateNewUnit), &createCreateNewUnitAction },
    { typeid(Actions::DeleteUnit), &createDeleteUnitAction },
    { typeid(Actions::AddNewAudioRoute), &createAddNewRouteAction },
    { typeid(Actions::AddNewMidiRoute), &createAddNewMidiRouteAction },
    { typeid(Actions::ModifyClipItem), &createModifyClipItemAction },

    //Timeline
    { typeid(Actions::ChangeSignatureBpm), &createChangeSignatureBpmAction },
    { typeid(Actions::ChangeTimelineState), &createChangeTimelineStateAction },
    { typeid(Actions::LoopPosition), &createLoopPositionAction },
    { typeid(Actions::ToggleLoop), &createToggleLoopAction },
    { typeid(Actions::UpdatePlayhead), &createUpdatePlayheadAction },

    //MidiController
    { typeid(Actions::ToggleMidiDevice), &createToggleMidiDeviceAction },
    { typeid(Actions::VMKTrigger), &createVMKTriggerAction },

    //Metronome
    { typeid(Actions::ToggleMetronome), &createToggleMetronomeAction }
};

}