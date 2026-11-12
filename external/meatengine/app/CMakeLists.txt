find_package(SFML 3.0 CONFIG REQUIRED COMPONENTS Graphics Window System)

add_library(meatengine_app STATIC
    src/app.cpp
    src/GameStateRegistry.cpp
    src/MainLoop.cpp
    src/systems/timer/update.cpp
)

target_include_directories(meatengine_app PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

meatengine_set_target_options(meatengine_app)

target_link_libraries(meatengine_app PUBLIC
    meatengine_assets
    meatengine_ecs
    SFML::Graphics SFML::Window SFML::System
)
