file(GLOB_RECURSE MEATENGINE_LUA_BINDINGS_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_library(meatengine_net_lua STATIC
    ${MEATENGINE_LUA_BINDINGS_SOURCES}
)

meatengine_set_target_options(meatengine_net_lua)

target_link_libraries(meatengine_net_lua PUBLIC
    meatengine_net
    meatengine_scripting
)

set_property(GLOBAL APPEND PROPERTY MEATENGINE_LUA_TARGETS meatengine_net_lua)