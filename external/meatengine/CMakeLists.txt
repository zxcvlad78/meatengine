cmake_minimum_required(VERSION 3.24)

project(meatengine VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_COMPILER "${CMAKE_CXX_COMPILER}" CACHE FILEPATH "" FORCE)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(meatengine_options)

find_package(GameNetworkingSockets CONFIG REQUIRED)

add_subdirectory(scripting/external/lua)

add_library(sol2 INTERFACE)
add_library(sol2::sol2 ALIAS sol2)

target_include_directories(sol2 INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/scripting/external/sol/include
)
target_link_libraries(sol2 INTERFACE Lua::Lua)

set(MEATENGINE_MODULES
    core
    abstract
    assets
    audio
    ecs
    spatial
    hierarchy
    physics
    net
    graphics
    ui
    app
    scripting
    modding
    debug
)

foreach(module IN LISTS MEATENGINE_MODULES)
    add_subdirectory(${module})
endforeach()

add_library(meatengine INTERFACE)
add_library(meatengine::meatengine ALIAS meatengine)

target_link_libraries(meatengine INTERFACE
    meatengine_core
    meatengine_abstract
    meatengine_assets
    meatengine_audio
    meatengine_ecs
    meatengine_spatial
    meatengine_hierarchy
    meatengine_physics
    meatengine_net
    meatengine_graphics
    meatengine_ui
    meatengine_app
    meatengine_scripting
    meatengine_modding
    meatengine_debug
)

set(MEATENGINE_MODULE_TARGETS
    meatengine_core
    meatengine_abstract
    meatengine_assets
    meatengine_audio
    meatengine_ecs
    meatengine_spatial
    meatengine_hierarchy
    meatengine_physics
    meatengine_net
    meatengine_graphics
    meatengine_ui
    meatengine_app
    meatengine_scripting
    meatengine_modding
    meatengine_debug
    PARENT_SCOPE
)
