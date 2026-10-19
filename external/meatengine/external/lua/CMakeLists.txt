file(GLOB LUA_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

list(FILTER LUA_SOURCES EXCLUDE REGEX ".*/lua\\.c$")
list(FILTER LUA_SOURCES EXCLUDE REGEX ".*/luac\\.c$")
list(FILTER LUA_SOURCES EXCLUDE REGEX ".*/onelua\\.c$")

add_library(lua_static STATIC ${LUA_SOURCES})

target_include_directories(lua_static PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (WIN32)
    target_compile_definitions(lua_static PRIVATE LUA_USE_WINDOWS)
endif()

set_target_properties(lua_static PROPERTIES LINKER_LANGUAGE C)

add_library(Lua::Lua ALIAS lua_static)