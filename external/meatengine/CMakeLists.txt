cmake_minimum_required(VERSION 3.24)

project(meatengine VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_COMPILER "${CMAKE_CXX_COMPILER}" CACHE FILEPATH "" FORCE)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(meatengine_options)

set(MEATENGINE_MODULES
    core
    abstract
    assets
    audio
    ecs
    spatial
    ui
    hierarchy
    physics
    net
    graphics
    app
    scripting
    modding
    debug
)

set(MEATENGINE_MODULE_REQUIRES "")
foreach(module IN LISTS MEATENGINE_MODULES)
    set(module_cmake "${CMAKE_CURRENT_SOURCE_DIR}/modules/${module}/CMakeLists.txt")
    if(EXISTS "${module_cmake}")
        file(READ "${module_cmake}" module_cmake_content)
        string(REGEX MATCHALL "meatengine_require\\([A-Za-z0-9_]+\\)" module_requires "${module_cmake_content}")
        foreach(module_require IN LISTS module_requires)
            string(REGEX REPLACE "^meatengine_require\\(([A-Za-z0-9_]+)\\)$" "\\1" module_dep "${module_require}")
            list(APPEND MEATENGINE_MODULE_REQUIRES "${module}:${module_dep}")
        endforeach()
    endif()
endforeach()

set(MEATENGINE_MODULES_ENABLED ${MEATENGINE_MODULES})
set(MEATENGINE_MODULES_PRUNED ON)
while(MEATENGINE_MODULES_PRUNED)
    set(MEATENGINE_MODULES_PRUNED OFF)
    foreach(module_require IN LISTS MEATENGINE_MODULE_REQUIRES)
        string(REPLACE ":" ";" module_require_parts "${module_require}")
        list(GET module_require_parts 0 module_name)
        list(GET module_require_parts 1 module_dep)
        if(module_name IN_LIST MEATENGINE_MODULES_ENABLED AND NOT module_dep IN_LIST MEATENGINE_MODULES_ENABLED)
            list(REMOVE_ITEM MEATENGINE_MODULES_ENABLED ${module_name})
            message(STATUS "meatengine: module '${module_name}' disabled (requires '${module_dep}')")
            set(MEATENGINE_MODULES_PRUNED ON)
        endif()
    endforeach()
endwhile()
set(MEATENGINE_MODULES ${MEATENGINE_MODULES_ENABLED})

if("scripting" IN_LIST MEATENGINE_MODULES)
    set(MEATENGINE_WITH_SCRIPTING ON)
else()
    set(MEATENGINE_WITH_SCRIPTING OFF)
endif()

if("net" IN_LIST MEATENGINE_MODULES)
    set(MEATENGINE_WITH_NET ON)
else()
    set(MEATENGINE_WITH_NET OFF)
endif()

if(MEATENGINE_WITH_SCRIPTING)
    add_subdirectory(modules/scripting/external/lua)

    add_library(sol2 INTERFACE)
    add_library(sol2::sol2 ALIAS sol2)

    target_include_directories(sol2 INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/scripting/external/sol/include
    )
    target_link_libraries(sol2 INTERFACE Lua::Lua)
endif()

if(MEATENGINE_WITH_NET)
    find_package(GameNetworkingSockets CONFIG REQUIRED)
endif()

foreach(module IN LISTS MEATENGINE_MODULES)
    add_subdirectory(modules/${module})
endforeach()

get_property(MEATENGINE_LUA_TARGETS GLOBAL PROPERTY MEATENGINE_LUA_TARGETS)

set(MEATENGINE_ALL_TARGETS "")
foreach(module IN LISTS MEATENGINE_MODULES)
    list(APPEND MEATENGINE_ALL_TARGETS meatengine_${module})
endforeach()
list(APPEND MEATENGINE_ALL_TARGETS ${MEATENGINE_LUA_TARGETS})

add_library(meatengine INTERFACE)
add_library(meatengine::meatengine ALIAS meatengine)

target_link_libraries(meatengine INTERFACE ${MEATENGINE_ALL_TARGETS})

set(MEATENGINE_MODULE_TARGETS ${MEATENGINE_ALL_TARGETS} PARENT_SCOPE)
