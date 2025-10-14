cmake_minimum_required(VERSION 3.14)
project(SLR LANGUAGES C CXX)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -fsanitize=address")
#set(CMAKE_CXX_FLAGS "-g -O0 -Wall -fsanitize=address")

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall")
set(CMAKE_CXX_FLAGS "-g -O0 -Wall")



if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type")
  set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall" CACHE STRING "Debug flags" FORCE)
endif()

message("Building ${CMAKE_BUILD_TYPE}")

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_package(SDL2 REQUIRED)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Sndfile REQUIRED)

pkg_check_modules(JACK REQUIRED IMPORTED_TARGET jack>=0.100.0)
find_library(JACK_LIBRARY NAMES ${JACK_LIBRARIES} HINTS ${JACK_LIBDIR})

include_directories(${PROJECT_SOURCE_DIR}/inc)
include_directories(${PROJECT_SOURCE_DIR}/shared)

#generate events and handles
add_custom_target(
    run_codegen
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/GenaV3/main.py
    COMMENT "Running code generator...(always)"
    # VERBATIM
)

add_subdirectory(external)

file(GLOB SOURCES
  #core
  inc/core/drivers/AudioDriver.h
  src/core/drivers/AudioDriver.cpp
  src/core/drivers/dummyDriver.cpp
  src/core/drivers/jackDriver.cpp

  src/core/primitives/AudioUnit.cpp
  src/core/primitives/AudioBuffer.cpp
  src/core/primitives/AudioFile.cpp
  src/core/primitives/AudioPeakFile.cpp
  src/core/primitives/AudioPeaks.cpp
  src/core/primitives/File.cpp
  src/core/primitives/MidiFile.cpp
  src/core/primitives/Parameter.cpp
  src/core/primitives/ParameterArray.cpp
  src/core/primitives/RenderPlan.cpp

  src/core/utility/basicAudioManipulation.cpp

  src/core/AudioBufferManager.cpp
  src/core/AudioPeakBuilder.cpp
  src/core/ControlEngine.cpp
  src/core/Events.cpp
  src/core/FileTasks.cpp
  src/core/FileWorker.cpp
  src/core/Mixer.cpp
  src/core/Project.cpp
  src/core/RtEngine.cpp
  src/core/SettingsManager.cpp
  src/core/Timeline.cpp
  src/core/Metronome.cpp

  src/core/ModuleManager.cpp

  #snapshots
  src/snapshots/AudioUnitView.cpp
  src/snapshots/FileContainerView.cpp
  src/snapshots/ParameterArrayView.cpp
  src/snapshots/ParameterView.cpp
  src/snapshots/ProjectView.cpp
  src/snapshots/TimelineView.cpp
  src/snapshots/DriverView.cpp

  #test
  modules/Track/Track.cpp
  modules/Track/TrackModule.cpp
  modules/Track/TrackUI.cpp
  modules/Track/TrackView.cpp

  modules/Mixer/Mixer.cpp
  modules/Mixer/MixerModule.cpp
  modules/Mixer/MixerUI.cpp
  modules/Mixer/MixerView.cpp

  #ui
  src/ui/display/primitives/BaseWidget.cpp
  src/ui/display/primitives/Button.cpp
  src/ui/display/primitives/Label.cpp
  src/ui/display/primitives/FileView.cpp
  src/ui/display/primitives/Popup.cpp
  src/ui/display/primitives/DropDown.cpp
  src/ui/display/primitives/UnitUIBase.cpp
  src/ui/display/primitives/View.cpp
  src/ui/display/primitives/UIContext.cpp

  src/ui/display/defaultStyles.cpp
  src/ui/display/defaultColors.cpp
  src/ui/display/SplashScreen.cpp
  src/ui/display/Timeline.cpp
  src/ui/display/TopPanel.cpp
  src/ui/display/BottomPanel.cpp
  src/ui/display/GridView.cpp
  src/ui/display/ModuleView.cpp
  src/ui/display/Browser.cpp
  src/ui/display/MainWindow.cpp
  src/ui/display/RouteManager.cpp
  src/ui/display/TimelinePopup.cpp
  src/ui/display/ScreenKeyboard.cpp
  src/ui/display/PopupManager.cpp
  src/ui/display/NewModulePopup.cpp

  src/ui/FakeGestures.cpp
  src/ui/uiControls.cpp
  src/ui/guiThread.cpp

  #other
  src/defines.cpp
)

#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)
#add_compile_options(-fsanitize=thread)
#add_link_options(-fsanitize=thread)

add_library(slrlib ${SOURCES})

add_dependencies(slrlib run_codegen)

add_executable(slr src/main.cpp)

set(LIBRARIES lvgl::lvgl
              SDL2::SDL2
              ${LIBSNDFILE_LIBRARIES}
              PkgConfig::JACK
              loguru::loguru
              GestLib)


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


