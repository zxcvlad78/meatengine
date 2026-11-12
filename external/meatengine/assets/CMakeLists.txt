find_package(EnTT CONFIG REQUIRED)
find_package(SFML 3.0 CONFIG REQUIRED COMPONENTS Graphics Audio)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(meatengine_assets STATIC
    src/assets.cpp
    src/ConfigFile.cpp
    src/FileSystem.cpp
    src/Resources.cpp
)

target_include_directories(meatengine_assets PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

meatengine_set_target_options(meatengine_assets)

target_link_libraries(meatengine_assets PUBLIC
    meatengine_core
    EnTT::EnTT
    SFML::Graphics SFML::Audio
    nlohmann_json::nlohmann_json
)
