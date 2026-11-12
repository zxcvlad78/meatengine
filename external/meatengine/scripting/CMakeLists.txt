file(GLOB_RECURSE MEATENGINE_LUA_BINDINGS_SOURCES CONFIGURE_DEPENDS "src/lua/bindings/*.cpp")

add_library(meatengine_scripting STATIC
    src/scripting.cpp
    src/ComponentDescriptor.cpp
    src/LuaBinder.cpp
    src/ScriptedStateRegistry.cpp
    src/ScriptingServer.cpp
    ${MEATENGINE_LUA_BINDINGS_SOURCES}
)

target_include_directories(meatengine_scripting PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

meatengine_set_target_options(meatengine_scripting)

target_link_libraries(meatengine_scripting PUBLIC
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
    sol2::sol2
)
