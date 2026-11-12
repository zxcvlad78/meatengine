find_package(SFML 3.0 CONFIG REQUIRED COMPONENTS System)

add_library(meatengine_spatial STATIC
    src/spatial.cpp
)

target_include_directories(meatengine_spatial PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

meatengine_set_target_options(meatengine_spatial)

target_link_libraries(meatengine_spatial PUBLIC
    SFML::System
)

if(MEATENGINE_WITH_SCRIPTING)
    add_subdirectory(lua)
endif()
