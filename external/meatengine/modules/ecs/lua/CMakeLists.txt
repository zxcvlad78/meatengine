file(GLOB_RECURSE MEATENGINE_LUA_BINDINGS_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_library(meatengine_ecs_lua STATIC
    ${MEATENGINE_LUA_BINDINGS_SOURCES}
)

meatengine_set_target_options(meatengine_ecs_lua)

target_link_libraries(meatengine_ecs_lua PUBLIC
    meatengine_ecs
    meatengine_scripting
)

set_property(GLOBAL APPEND PROPERTY MEATENGINE_LUA_TARGETS meatengine_ecs_lua)