cmake_minimum_required(VERSION 3.14)
project(SLR LANGUAGES C CXX)

#options
option(TESTING "build tests" OFF)

#loguru doesn't support -fno-exceptions flag...
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Werror=return-type -Wextra")
set(CMAKE_CXX_FLAGS "-g -O0 -Wall -Werror=return-type -Wextra")
message("Build: ${CMAKE_BUILD_TYPE}")
message("Flags: ${CMAKE_CXX_FLAGS}")

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

#find all packages
find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_package(SDL2 REQUIRED)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Sndfile REQUIRED)
find_package(ALSA)

pkg_check_modules(JACK REQUIRED IMPORTED_TARGET jack>=0.100.0)
find_library(JACK_LIBRARY NAMES ${JACK_LIBRARIES} HINTS ${JACK_LIBDIR})

# Find libusb
find_path(LIBUSB_INCLUDE_DIR NAMES libusb.h PATH_SUFFIXES "include" "libusb" "libusb-1.0")
find_library(LIBUSB_LIBRARY NAMES usb usb-1.0 PATH_SUFFIXES "lib" "lib32" "lib64")

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

### Grab all our units to compile ### (leave it, future improvement)
#grab all targets
#set(FXTARGETS )
#get_property(_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/units PROPERTY SUBDIRECTORIES)
#foreach(_dir IN LISTS _dirs)
#    get_property(FX DIRECTORY ${_dir} PROPERTY BUILDSYSTEM_TARGETS)
#    list(APPEND FXTARGETS "${FX}")
#endforeach()

#message("List of units to build:")
#message("${FXTARGETS}")

#gather sources
set_property(GLOBAL PROPERTY SOURCES "")

add_subdirectory(src)
add_subdirectory(units)

get_property(SOURCES GLOBAL PROPERTY SOURCES)


#compile target
add_library(slrlib ${SOURCES})
target_link_libraries(slrlib ${LIBRARIES})
list(APPEND LIBRARIES slrlib)

add_executable(slr src/main.cpp)
target_sources(slr PRIVATE ${SOURCES})
target_link_libraries(slr ${LIBRARIES})

if(TESTING)
  message("Testing enabled")

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
else()
  message("Building without tests")
endif()


