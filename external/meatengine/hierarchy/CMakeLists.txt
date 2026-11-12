find_package(EnTT CONFIG REQUIRED)

add_library(meatengine_hierarchy STATIC
    src/hierarchy.cpp
    src/systems/hierarchy/update.cpp
)

target_include_directories(meatengine_hierarchy PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

meatengine_set_target_options(meatengine_hierarchy)

target_link_libraries(meatengine_hierarchy PUBLIC
    meatengine_abstract
    meatengine_app
    meatengine_ecs
    meatengine_spatial
    EnTT::EnTT
)
