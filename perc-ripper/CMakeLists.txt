cmake_minimum_required(VERSION 3.16)

project(PercRipper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_VS_PLATFORM_NAME "x64")
endif()

if(NOT CMAKE_VS_PLATFORM_NAME STREQUAL "x64" AND
        NOT CMAKE_VS_PLATFORM_NAME STREQUAL "x86")
    message(FATAL_ERROR "${CMAKE_VS_PLATFORM_NAME} architecture is not supported!")
endif()

set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE STRING "" FORCE)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(FetchContent)

FetchContent_Declare(
        Zydis
        GIT_REPOSITORY https://github.com/zyantific/zydis.git
        GIT_TAG v4.1.1
        GIT_SHALLOW TRUE
)

set(ZYDIS_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(ZYDIS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ZYDIS_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(Zydis)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/PercRipper/CMakeLists.txt")
    add_subdirectory(PercRipper)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/dump/CMakeLists.txt")
    add_subdirectory(dump PercRipper)
else()
    message(FATAL_ERROR "Could not find PercRipper or dump directory!")
endif()
