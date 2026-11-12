add_library(meatengine_physics STATIC
    src/physics.cpp
    src/systems/movement.cpp
    src/systems/collision.cpp
)

target_include_directories(meatengine_physics PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

meatengine_set_target_options(meatengine_physics)

target_link_libraries(meatengine_physics PUBLIC
    meatengine_app
    meatengine_ecs
    meatengine_spatial
    EnTT
)

meatengine_require(app)
meatengine_require(ecs)
meatengine_require(spatial)

if(MEATENGINE_WITH_SCRIPTING)
    add_subdirectory(lua)
endif()
