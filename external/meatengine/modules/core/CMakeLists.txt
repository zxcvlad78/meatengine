find_package(SFML 3.0 CONFIG REQUIRED COMPONENTS Graphics)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(meatengine_core STATIC
    src/core.cpp
    src/bbcode.cpp
)

target_include_directories(meatengine_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

meatengine_set_target_options(meatengine_core)

target_link_libraries(meatengine_core PUBLIC
    SFML::Graphics
    nlohmann_json::nlohmann_json
)
