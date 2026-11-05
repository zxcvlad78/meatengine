cmake_minimum_required(VERSION 3.20)

project(meatengine VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_COMPILER "${CMAKE_CXX_COMPILER}" CACHE FILEPATH "" FORCE)

find_package(SFML 3.0 CONFIG REQUIRED COMPONENTS Graphics Window System Audio)
find_package(EnTT CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(GameNetworkingSockets CONFIG REQUIRED)

add_subdirectory(external/lua)

add_library(sol2 INTERFACE)
add_library(sol2::sol2 ALIAS sol2)

target_include_directories(sol2 INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/sol/include
)

target_link_libraries(sol2 INTERFACE Lua::Lua)

file(GLOB_RECURSE MEATENGINE_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE MEATENGINE_HEADERS CONFIGURE_DEPENDS "include/*.hpp")

add_library(meatengine ${MEATENGINE_SOURCES} ${MEATENGINE_HEADERS})
add_library(meatengine::meatengine ALIAS meatengine)

target_include_directories(meatengine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(meatengine PUBLIC
    SFML::Graphics SFML::Window SFML::System SFML::Audio
	GameNetworkingSockets::GameNetworkingSockets
    EnTT::EnTT
    nlohmann_json::nlohmann_json
    sol2::sol2
)

