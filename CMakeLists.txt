cmake_minimum_required(VERSION 3.14)
project(SLR LANGUAGES C CXX)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -fsanitize=address")
#set(CMAKE_CXX_FLAGS "-g -O0 -Wall -fsanitize=address")

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Werror=return-type")
set(CMAKE_CXX_FLAGS "-g -O0 -Wall -Werror=return-type")



if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type")
  set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Werror=return-type" CACHE STRING "Debug flags" FORCE)
endif()

message("Building ${CMAKE_BUILD_TYPE}")

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_package(SDL2 REQUIRED)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Sndfile REQUIRED)
find_package(ALSA)
#pkg_check_modules(UDEV REQUIRED libudev)

pkg_check_modules(JACK REQUIRED IMPORTED_TARGET jack>=0.100.0)
find_library(JACK_LIBRARY NAMES ${JACK_LIBRARIES} HINTS ${JACK_LIBDIR})

# Find libusb
find_path(LIBUSB_INCLUDE_DIR
  NAMES libusb.h
  PATH_SUFFIXES "include" "libusb" "libusb-1.0")
find_library(LIBUSB_LIBRARY
  NAMES usb usb-1.0
  PATH_SUFFIXES "lib" "lib32" "lib64")

#message(STATUS ${LIBUSB_INCLUDE_DIR})
#message(STATUS ${LIBUSB_LIBRARY})

if((NOT LIBUSB_INCLUDE_DIR) OR (NOT LIBUSB_LIBRARY))
  message(FATAL_ERROR "Can't find libusb. Please verify that it is installed")
endif()


include_directories(${PROJECT_SOURCE_DIR}/src)

add_subdirectory(external)


set(LIBRARIES lvgl::lvgl
              SDL2::SDL2
              ${LIBSNDFILE_LIBRARIES}
              PkgConfig::JACK
              loguru::loguru
              GestLib
              ALSA::ALSA
              ${LIBUSB_LIBRARY})


### Grab all our units to compile ###
#add_subdirectory(units)

#grab all targets
#set(FXTARGETS )
#get_property(_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/units PROPERTY SUBDIRECTORIES)
#foreach(_dir IN LISTS _dirs)
#    get_property(FX DIRECTORY ${_dir} PROPERTY BUILDSYSTEM_TARGETS)
#    list(APPEND FXTARGETS "${FX}")
#endforeach()

#message("List of units to build:")
#message("${FXTARGETS}")

file(GLOB SOURCES
  #actions
  src/core/actions/AudioUnit/LoadAsClip.cpp
  src/core/actions/AudioUnit/RemoveClip.cpp
  src/core/actions/AudioUnit/SetParameter.cpp
  src/core/actions/AudioUnit/ToggleMidiThru.cpp
  src/core/actions/AudioUnit/ToggleOmniHwInput.cpp
  src/core/actions/AudioUnit/SetName.cpp
  src/core/actions/AudioUnit/SetColor.cpp

  src/core/actions/Project/CreateNewUnit.cpp
  src/core/actions/Project/DeleteUnit.cpp
  src/core/actions/Project/AddNewAudioRoute.cpp
  src/core/actions/Project/AddNewMidiRoute.cpp
  src/core/actions/Project/ModifyClipItem.cpp
  src/core/actions/Project/SaveProject.cpp
  src/core/actions/Project/LoadProject.cpp
  src/core/actions/Project/UpdateRenderPlan.cpp
  src/core/actions/Project/Undo.cpp
  src/core/actions/Project/Redo.cpp

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

  #filetasks
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
  src/core/primitives/MidiPort.cpp
  src/core/primitives/Parameter.cpp
  src/core/primitives/ParameterArray.cpp
  src/core/primitives/RtTask.cpp

  #utility
  src/core/utility/basicAudioManipulation.cpp
  src/core/utility/basicMidiManipulation.cpp
  src/core/utility/ControlContext.cpp
  src/core/utility/FileWorkerContext.cpp

  #core
  src/core/AudioPeakBuilder.cpp
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

  #snapshots
  src/snapshots/AudioUnitView.cpp
  src/snapshots/FileContainerView.cpp
  src/snapshots/ParameterArrayView.cpp
  src/snapshots/ParameterView.cpp
  src/snapshots/ProjectView.cpp
  src/snapshots/TimelineView.cpp
  src/snapshots/DriverView.cpp
  src/snapshots/SequenceView.cpp
  src/snapshots/ModulationView.cpp

  #test
  units/Track/Track.cpp
  units/Track/TrackUnit.cpp
  units/Track/TrackUI.cpp
  units/Track/TrackView.cpp
  units/Track/TrackPushUI.cpp
  units/Track/TrackActions.cpp
  units/Track/Actions/DumpRecordedAudio.cpp
  units/Track/Actions/RecordArm.cpp
  units/Track/Actions/ReinitTrackRecord.cpp

  units/Mixer/Mixer.cpp
  units/Mixer/MixerUnit.cpp
  units/Mixer/MixerUI.cpp
  units/Mixer/MixerView.cpp
  units/Mixer/MixerPushUI.cpp

  units/SimpleOscillator/SimpleOsc.cpp
  units/SimpleOscillator/SimpleOscUnit.cpp
  units/SimpleOscillator/SimpleOscUI.cpp
  units/SimpleOscillator/SimpleOscView.cpp
  units/SimpleOscillator/SimpleOscPushUI.cpp

  units/Sampler/Sampler.cpp
  units/Sampler/SamplerUnit.cpp
  units/Sampler/SamplerView.cpp
  units/Sampler/SamplerUI.cpp
  units/Sampler/SamplerPushUI.cpp
  units/Sampler/Actions/LoadAsset.cpp

  #ui
  src/display/elements/BottomPanel.cpp
  src/display/elements/Browser.cpp
  src/display/elements/GridView.cpp
  src/display/elements/MainWindow.cpp
  src/display/elements/ModEngineView.cpp
  src/display/elements/SplashScreen.cpp
  src/display/elements/StepSequencerView.cpp
  src/display/elements/Timeline.cpp
  src/display/elements/TopPanel.cpp
  src/display/elements/UnitView.cpp
  
  src/display/popups/DialogWindows.cpp
  src/display/popups/DragViewSelector.cpp
  src/display/popups/ModEngineTargetManager.cpp
  src/display/popups/NewUnitPopup.cpp
  src/display/popups/PopupManager.cpp
  src/display/popups/RouteManager.cpp
  src/display/popups/ScreenKeyboard.cpp
  src/display/popups/SettingsPopup.cpp
  src/display/popups/TimelinePopup.cpp
  src/display/popups/VirtualMidiKeyboard.cpp

  src/display/primitives/BaseWidget.cpp
  src/display/primitives/Button.cpp
  src/display/primitives/Checkbox.cpp
  src/display/primitives/DropDown.cpp
  src/display/primitives/FileView.cpp
  src/display/primitives/Label.cpp
  src/display/primitives/Popup.cpp
  src/display/primitives/Slider.cpp
  src/display/primitives/UnitUIBase.cpp
  src/display/primitives/View.cpp

  src/display/utility/AudioFileToCanvas.cpp
  src/display/utility/defaultColors.cpp
  src/display/utility/defaultStyles.cpp
  src/display/utility/DragContext.cpp
  src/display/utility/UIContext.cpp

  src/display/FakeGestures.cpp
  src/display/guiThread.cpp

  #push core
  src/push/core/PushContext.cpp
  src/push/core/PushCore.cpp
  src/push/core/PushDisplay.cpp
  src/push/core/PushFaker.cpp
  src/push/core/PushFonts.cpp
  src/push/core/PushLeds.cpp
  src/push/core/PushMidi.cpp
  src/push/core/PushPads.cpp
  src/push/core/PushPainter.cpp
  src/push/core/PushSysex.cpp
  src/push/core/Widget.cpp

  #widgets
  src/push/elements/BrowserWidget.cpp
  src/push/elements/GridWidget.cpp
  src/push/elements/PadLayoutWidget.cpp
  src/push/elements/RootWidget.cpp
  src/push/elements/UnitWidget.cpp

  #primitives
  src/push/primitives/Canvas.cpp
  src/push/primitives/Label.cpp
  src/push/primitives/Pointer.cpp
  src/push/primitives/Rectangle.cpp
  src/push/primitives/StaticLabel.cpp
  src/push/primitives/UnitUIBase.cpp

  #utility
  src/push/utility/PadNoteLayout.cpp
  src/push/utility/PushUIContext.cpp

  src/push/pushThread.cpp

  #common
  src/common/defines.cpp
  src/common/uiControls.cpp
  src/common/Profiler.cpp
)

#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)
#add_compile_options(-fsanitize=thread)
#add_link_options(-fsanitize=thread)

add_library(slrlib ${SOURCES})

#add_dependencies(slrlib run_codegen)

add_executable(slr src/main.cpp)

target_link_libraries(slrlib ${LIBRARIES})


list(APPEND LIBRARIES slrlib)
target_link_libraries(slr ${LIBRARIES})

target_include_directories(slr PUBLIC ${CMAKE_SOURCE_DIR}/inc)

message("Flags: ${CMAKE_CXX_FLAGS}")

if(NOT TESTING)
  message("Testing enabled")
  set(TESTING true)

  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0" CACHE STRING "Debug flags" FORCE)
  endif()
  
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/heads/main.zip
  )

  # Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  find_package(GTest REQUIRED)
  include(cmake/tests.cmake)
  enable_testing()
  add_subdirectory(tests)
endif()



if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(slr PRIVATE -O0 -g -Wall)
    message(STATUS "Debug flags enabled")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(slr PRIVATE -O3)
    target_compile_definitions(slr PRIVATE NDEBUG)
    message(STATUS "Release flags enabled")
endif()


