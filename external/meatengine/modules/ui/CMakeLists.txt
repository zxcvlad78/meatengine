find_package(SFML 3.0 CONFIG REQUIRED COMPONENTS Graphics)

add_library(meatengine_ui STATIC
    src/ui.cpp
    src/helpers.cpp
    src/setup/ui.cpp
    src/systems/ui/bounds.cpp
    src/systems/ui/focus.cpp
    src/systems/ui/interaction.cpp
    src/systems/ui/layout.cpp
    src/systems/ui/process_events.cpp
    src/systems/ui/render.cpp
    src/systems/ui/stack.cpp
    src/systems/ui/style.cpp
)

target_include_directories(meatengine_ui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

meatengine_set_target_options(meatengine_ui)

target_link_libraries(meatengine_ui PUBLIC
    meatengine_abstract
    meatengine_assets
    meatengine_app
    meatengine_ecs
    meatengine_hierarchy
    meatengine_spatial
    SFML::Graphics
)

meatengine_require(abstract)
meatengine_require(app)
meatengine_require(assets)
meatengine_require(ecs)
meatengine_require(hierarchy)
meatengine_require(spatial)

if(MEATENGINE_WITH_SCRIPTING)
    add_subdirectory(lua)
endif()
