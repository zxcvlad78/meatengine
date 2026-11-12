add_library(meatengine_modding STATIC
    src/modding.cpp
    src/ModLoader.cpp
    src/hooks.cpp
    src/lua/bindings/mod_info.cpp
    src/lua/bindings/mod_loader.cpp
)

target_include_directories(meatengine_modding PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

meatengine_set_target_options(meatengine_modding)

target_link_libraries(meatengine_modding PUBLIC
    meatengine_app
    meatengine_scripting
)
