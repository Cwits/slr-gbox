cmake_minimum_required(VERSION 3.15)

#set(PROJECTNAME "Track")
#project (${PROJECTNAME} VERSION 0.0.1)

#include(GenerateExportHeader)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/")

set(SOURCES 
    Track.cpp 
    TrackActions.cpp
    TrackPushUI.cpp
    TrackUI.cpp
    TrackUnit.cpp
    TrackView.cpp
    Actions/DumpRecordedAudio.cpp
    Actions/RecordArm.cpp
    Actions/ReinitTrackRecord.cpp)


#add_library(${PROJECTNAME} SHARED ${SOURCES})

#target_link_libraries(${PROJECTNAME} ${LIBRARIES})

#GENERATE_EXPORT_HEADER(${PROJECTNAME} BASE_NAME "PLUGIN")

#target_include_directories(${PROJECTNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#add_custom_command(TARGET ${PROJECTNAME}
#                   POST_BUILD
#                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECTNAME}> /home/pi/Desktop/plugins)
# something with ${CMAKE_CURRENT_BINARY_DIR}

