find_package(SFML 3.0 CONFIG REQUIRED COMPONENTS Graphics Window System)
find_package(glm CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

add_library(meatengine_3d STATIC
    src/three_d.cpp
    src/MeshOps.cpp
    src/MeshGPU.cpp
    src/MeshIO.cpp
    src/Primitives.cpp
    src/ShaderOps.cpp
    src/Shader3D.cpp
    src/DefaultShader.cpp
    src/Math.cpp
    src/RenderState.cpp
    src/CameraOps.cpp
    src/RendererOps.cpp
    src/MaterialOps.cpp
    src/systems/transform3d.cpp
    src/systems/render3d.cpp
)

target_include_directories(meatengine_3d PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

meatengine_set_target_options(meatengine_3d)

target_link_libraries(meatengine_3d PUBLIC
    meatengine_abstract
    meatengine_assets
    meatengine_ecs
    meatengine_app
    meatengine_hierarchy
    SFML::Graphics SFML::Window SFML::System
    glm::glm
    glad::glad
)

meatengine_require(abstract)
meatengine_require(app)
meatengine_require(assets)
meatengine_require(ecs)
meatengine_require(hierarchy)

if(MEATENGINE_WITH_SCRIPTING)
    add_subdirectory(lua)
endif()
