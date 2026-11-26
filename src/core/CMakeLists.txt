
set_property(GLOBAL APPEND PROPERTY SOURCES

  #actions
  src/core/actions/AudioUnit/SetParameter.cpp
  src/core/actions/AudioUnit/ToggleMidiThru.cpp
  src/core/actions/AudioUnit/ToggleOmniHwInput.cpp
  src/core/actions/AudioUnit/SetName.cpp
  src/core/actions/AudioUnit/SetColor.cpp

  src/core/actions/ClipItem/LoadAsClip.cpp
  src/core/actions/ClipItem/RemoveClip.cpp
  src/core/actions/ClipItem/ModifyClipItem.cpp

  src/core/actions/Project/CreateNewUnit.cpp
  src/core/actions/Project/DeleteUnit.cpp
  src/core/actions/Project/AddNewAudioRoute.cpp
  src/core/actions/Project/AddNewMidiRoute.cpp
  src/core/actions/Project/SaveProject.cpp
  src/core/actions/Project/LoadProject.cpp
  src/core/actions/Project/UpdateRenderPlan.cpp
  src/core/actions/Project/Undo.cpp
  src/core/actions/Project/Redo.cpp

  src/core/actions/FileTaskProxy/CopyAudio.cpp
  src/core/actions/FileTaskProxy/PasteAudio.cpp
  src/core/actions/FileTaskProxy/DeleteAudio.cpp
  src/core/actions/FileTaskProxy/CutAudio.cpp
  src/core/actions/FileTaskProxy/SilenceAudio.cpp
  src/core/actions/FileTaskProxy/ReverseAudio.cpp
  src/core/actions/FileTaskProxy/AddNewMidiNote.cpp
  src/core/actions/FileTaskProxy/EraseMidiNotes.cpp

  src/core/actions/Timeline/ChangeSignatureBpm.cpp
  src/core/actions/Timeline/ChangeTimelineState.cpp
  src/core/actions/Timeline/LoopPosition.cpp
  src/core/actions/Timeline/ToggleLoop.cpp
  src/core/actions/Timeline/UpdatePlayhead.cpp
  
  src/core/actions/MidiController/ToggleMidiDevice.cpp
  src/core/actions/MidiController/VirtualMidiKbdAction.cpp
  
  src/core/actions/Metronome/ToggleMetronome.cpp

  src/core/actions/StepSequencer/CreateNewSequence.cpp
  src/core/actions/StepSequencer/ModifySequence.cpp
  src/core/actions/StepSequencer/ModifySequenceLayer.cpp
  src/core/actions/StepSequencer/ModifySequenceEvent.cpp
  src/core/actions/StepSequencer/ModifySequenceTarget.cpp

  src/core/actions/ModulationEngine/CreateNewModulation.cpp
  src/core/actions/ModulationEngine/ModifyModulation.cpp
  src/core/actions/ModulationEngine/ModifyModulationTarget.cpp

  src/core/actions/Actions.cpp
  src/core/actions/ActionsMap.cpp

  #drivers
  src/core/drivers/AudioDriver.cpp
  src/core/drivers/dummyDriver.cpp
  src/core/drivers/jackDriver.cpp
  src/core/drivers/udpDriver.cpp

  #filetasks
  src/core/filetasks/tasks/OpenFile.cpp
  src/core/filetasks/tasks/DumpAudio.cpp
  src/core/filetasks/tasks/CopyAudio.cpp
  src/core/filetasks/tasks/PasteAudio.cpp
  src/core/filetasks/tasks/UpdateAudioPeaks.cpp
  src/core/filetasks/tasks/DeleteAudio.cpp
  src/core/filetasks/tasks/CutAudio.cpp
  src/core/filetasks/tasks/SilenceAudio.cpp
  src/core/filetasks/tasks/ReverseAudio.cpp
  src/core/filetasks/tasks/AddNewMidiNoteTask.cpp
  src/core/filetasks/tasks/UpdateMidiPeaks.cpp
  src/core/filetasks/tasks/EraseMidiNotesTask.cpp

  src/core/filetasks/FileTasks.cpp

  #primitives
  src/core/primitives/AudioBuffer.cpp
  src/core/primitives/AudioFile.cpp
  src/core/primitives/AudioPeakFile.cpp
  src/core/primitives/AudioPeaks.cpp
  src/core/primitives/AudioUnit.cpp
  src/core/primitives/File.cpp
  src/core/primitives/ClipItem.cpp
  src/core/primitives/MidiFile.cpp
  src/core/primitives/MidiPeaks.cpp
  src/core/primitives/MidiPort.cpp
  src/core/primitives/Parameter.cpp
  src/core/primitives/ParameterArray.cpp
  src/core/primitives/RtTask.cpp
  src/core/primitives/IdGenerator.cpp

  #utility
  src/core/utility/basicAudioManipulation.cpp
  src/core/utility/basicMidiManipulation.cpp
  src/core/utility/ControlContext.cpp
  src/core/utility/FileWorkerContext.cpp

  #core
  src/core/BufferManager.cpp
  src/core/ControlEngine.cpp
  src/core/FileWorker.cpp
  src/core/Metronome.cpp
  src/core/MidiController.cpp
  src/core/ModulationEngine.cpp
  src/core/Project.cpp
  src/core/RenderPlan.cpp
  src/core/RtEngine.cpp
  src/core/Serializer.cpp       #move to utility
  src/core/SettingsManager.cpp
  src/core/StepSequencer.cpp
  src/core/Timeline.cpp
  src/core/UnitManager.cpp


)