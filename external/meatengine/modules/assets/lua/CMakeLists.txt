file(GLOB_RECURSE MEATENGINE_LUA_BINDINGS_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_library(meatengine_assets_lua STATIC
    ${MEATENGINE_LUA_BINDINGS_SOURCES}
)

target_include_directories(meatengine_assets_lua PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

meatengine_set_target_options(meatengine_assets_lua)

target_link_libraries(meatengine_assets_lua PUBLIC
    meatengine_assets
    meatengine_scripting
)

set_property(GLOBAL APPEND PROPERTY MEATENGINE_LUA_TARGETS meatengine_assets_lua)
