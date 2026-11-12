find_package(EnTT CONFIG REQUIRED)

add_library(meatengine_ecs STATIC
    src/ecs.cpp
    src/PackedEntity.cpp
    src/registry_setup.cpp
    src/SystemRegistry.cpp
)

target_include_directories(meatengine_ecs PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

meatengine_set_target_options(meatengine_ecs)

target_link_libraries(meatengine_ecs PUBLIC
    EnTT::EnTT
)

if(MEATENGINE_WITH_SCRIPTING)
    add_subdirectory(lua)
endif()
