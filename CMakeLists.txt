cmake_minimum_required(VERSION 3.20)
project(motionfx VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(include)

# Find or specify libobs headers
set(LIBOBS_INCLUDE_DIRS "")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/obs-repo/libobs/obs.h")
    set(LIBOBS_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/obs-repo/libobs")
elseif(EXISTS "/usr/include/obs/obs.h")
    set(LIBOBS_INCLUDE_DIRS "/usr/include/obs")
elseif(EXISTS "/usr/local/include/obs/obs.h")
    set(LIBOBS_INCLUDE_DIRS "/usr/local/include/obs")
endif()

if(LIBOBS_INCLUDE_DIRS)
    include_directories(${LIBOBS_INCLUDE_DIRS})
endif()

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

# Core static library (for standalone testing / CLI embedding)
add_library(motionfx_core STATIC ${MOTIONFX_SOURCES})

# Test executables (optional, enabled with -DBUILD_TESTS=ON)
option(BUILD_TESTS "Build test suite" OFF)
if(BUILD_TESTS)
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
endif()

# OBS Plugin Module
add_library(motionfx MODULE
    src/motionfx_plugin.cpp
    ${MOTIONFX_SOURCES}
)
target_compile_definitions(motionfx PRIVATE OBS_PLUGIN_BUILD=1)
set_target_properties(motionfx PROPERTIES PREFIX "")

# Platform-specific linking and configuration
if(WIN32)
    set_target_properties(motionfx PROPERTIES SUFFIX ".dll")
    if(MSVC)
        if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/obs.lib")
            target_link_libraries(motionfx PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/obs.lib")
        elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/obs.lib")
            target_link_libraries(motionfx PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/obs.lib")
        endif()
    elseif(MINGW)
        target_link_options(motionfx PRIVATE -static-libgcc -static-libstdc++)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/libobs.a")
            target_link_libraries(motionfx PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/libobs.a")
        elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libobs.a")
            target_link_libraries(motionfx PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libobs.a")
        endif()
    endif()

elseif(APPLE)
    set_target_properties(motionfx PROPERTIES
        SUFFIX ".so"
    )
    target_link_options(motionfx PRIVATE "-undefined" "dynamic_lookup")

elseif(UNIX)
    set_target_properties(motionfx PROPERTIES SUFFIX ".so")
    find_library(LIBOBS_LIB NAMES obs libobs PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)
    if(LIBOBS_LIB)
        target_link_libraries(motionfx PRIVATE ${LIBOBS_LIB})
    else()
        target_link_libraries(motionfx PRIVATE obs)
    endif()
endif()
