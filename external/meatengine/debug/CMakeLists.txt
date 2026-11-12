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

target_link_libraries(meatengine_debug PUBLIC
    meatengine_core
    meatengine_assets
    meatengine_app
    meatengine_scripting
    SFML::Graphics SFML::Window SFML::Audio
)
