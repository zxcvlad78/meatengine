file(GLOB_RECURSE MEATENGINE_DEBUG_LUA_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_library(meatengine_debug_lua STATIC
    ${MEATENGINE_DEBUG_LUA_SOURCES}
)

meatengine_set_target_options(meatengine_debug_lua)

target_link_libraries(meatengine_debug_lua PUBLIC
    meatengine_debug
    meatengine_scripting
)

set_property(GLOBAL APPEND PROPERTY MEATENGINE_LUA_TARGETS meatengine_debug_lua)
