find_package(SFML 3.0 CONFIG REQUIRED COMPONENTS Graphics Window System Audio)

add_library(meatengine_graphics STATIC
    src/graphics.cpp
    src/systems/camera/update.cpp
    src/systems/full_screen_scale.cpp
    src/systems/sprite/render.cpp
    src/systems/sprite/update.cpp
    src/systems/sprite_animation/update.cpp
    src/systems/tilemap/render.cpp
)

target_include_directories(meatengine_graphics PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

meatengine_set_target_options(meatengine_graphics)

target_link_libraries(meatengine_graphics PUBLIC
    meatengine_abstract
    meatengine_assets
    meatengine_ecs
    meatengine_spatial
    meatengine_app
    SFML::Graphics SFML::Window SFML::System SFML::Audio
)

if(MEATENGINE_WITH_NET)
    target_compile_definitions(meatengine_graphics PRIVATE MEATENGINE_WITH_NET)
    target_link_libraries(meatengine_graphics PRIVATE meatengine_net)
endif()

meatengine_require(abstract)
meatengine_require(app)
meatengine_require(assets)
meatengine_require(ecs)
meatengine_require(spatial)

if(MEATENGINE_WITH_SCRIPTING)
    add_subdirectory(lua)
endif()
