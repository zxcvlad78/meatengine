file(GLOB_RECURSE MEATENGINE_LUA_BINDINGS_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_library(meatengine_physics_lua STATIC
    ${MEATENGINE_LUA_BINDINGS_SOURCES}
)

meatengine_set_target_options(meatengine_physics_lua)

target_link_libraries(meatengine_physics_lua PUBLIC
    meatengine_physics
    meatengine_scripting
)

set_property(GLOBAL APPEND PROPERTY MEATENGINE_LUA_TARGETS meatengine_physics_lua)