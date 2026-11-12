file(GLOB_RECURSE MEATENGINE_SF_BINDINGS_SOURCES CONFIGURE_DEPENDS "src/lua/bindings/sf/*.cpp")

add_library(meatengine_scripting STATIC
    src/scripting.cpp
    src/ComponentDescriptor.cpp
    src/LuaBinder.cpp
    src/ScriptedStateRegistry.cpp
    src/ScriptingServer.cpp
    src/lua/bindings/api/lua_binder.cpp
    src/lua/bindings/api/scripted_state_registry.cpp
    ${MEATENGINE_SF_BINDINGS_SOURCES}
)

target_include_directories(meatengine_scripting PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

meatengine_set_target_options(meatengine_scripting)

meatengine_require(app)

target_link_libraries(meatengine_scripting PUBLIC
    meatengine_app
    sol2::sol2
)
