file(GLOB_RECURSE MEATENGINE_LUA_BINDINGS_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_library(meatengine_graphics_lua STATIC
    ${MEATENGINE_LUA_BINDINGS_SOURCES}
)

meatengine_set_target_options(meatengine_graphics_lua)

target_link_libraries(meatengine_graphics_lua PUBLIC
    meatengine_graphics
    meatengine_scripting
)

set_property(GLOBAL APPEND PROPERTY MEATENGINE_LUA_TARGETS meatengine_graphics_lua)