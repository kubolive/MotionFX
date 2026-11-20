cmake_minimum_required(VERSION 3.20)
project(motionfx VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# Core engine sources
set(MOTIONFX_SOURCES
    src/motionfx_easing.cpp
    src/motionfx_timeline.cpp
    src/motionfx_tracks.cpp
    src/motionfx_engine.cpp
    src/motionfx_3d.cpp
    src/motionfx_effects.cpp
    src/motionfx_presets.cpp
    src/motionfx_dispatcher.cpp
)

# Core static library
add_library(motionfx_core STATIC ${MOTIONFX_SOURCES})

# Test Executables
add_executable(test_easing tests/test_easing.cpp)
target_link_libraries(test_easing PRIVATE motionfx_core)

add_executable(test_timeline tests/test_timeline.cpp)
target_link_libraries(test_timeline PRIVATE motionfx_core)

add_executable(test_composable tests/test_composable.cpp)
target_link_libraries(test_composable PRIVATE motionfx_core)

add_executable(test_engine tests/test_engine.cpp)
target_link_libraries(test_engine PRIVATE motionfx_core)

add_executable(test_3d_motion tests/test_3d_motion.cpp)
target_link_libraries(test_3d_motion PRIVATE motionfx_core)

add_executable(test_effects tests/test_effects.cpp)
target_link_libraries(test_effects PRIVATE motionfx_core)

add_executable(test_dispatcher tests/test_dispatcher.cpp)
target_link_libraries(test_dispatcher PRIVATE motionfx_core)

add_executable(test_master_suite tests/test_master_suite.cpp)
target_link_libraries(test_master_suite PRIVATE motionfx_core)

# OBS Plugin Shared Library (if libobs headers are present)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/obs-repo/libobs/obs.h")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/obs-repo/libobs)
    add_library(motionfx MODULE
        src/motionfx_plugin.cpp
        ${MOTIONFX_SOURCES}
    )
    target_compile_definitions(motionfx PRIVATE OBS_PLUGIN_BUILD=1)
    set_target_properties(motionfx PROPERTIES PREFIX "")
endif()
