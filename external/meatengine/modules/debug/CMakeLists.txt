find_package(SFML 3.0 CONFIG REQUIRED COMPONENTS Graphics Window Audio)

add_library(meatengine_debug STATIC
    src/debug.cpp
    src/Console.cpp
    src/hooks.cpp
)

target_include_directories(meatengine_debug PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

meatengine_set_target_options(meatengine_debug)

meatengine_require(core)
meatengine_require(assets)
meatengine_require(app)

target_link_libraries(meatengine_debug PUBLIC
    meatengine_core
    meatengine_assets
    meatengine_app
    SFML::Graphics SFML::Window SFML::Audio
)

if(MEATENGINE_WITH_SCRIPTING)
    add_subdirectory(lua)
endif()
