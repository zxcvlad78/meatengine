add_library(meatengine_modding STATIC
    src/modding.cpp
    src/ModLoader.cpp
    src/hooks.cpp
)

target_include_directories(meatengine_modding PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

meatengine_set_target_options(meatengine_modding)

target_link_libraries(meatengine_modding PUBLIC
    meatengine_app
    meatengine_scripting
)

meatengine_require(app)
meatengine_require(scripting)

if(MEATENGINE_WITH_SCRIPTING)
    add_subdirectory(lua)
endif()
