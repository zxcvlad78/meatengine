find_package(SFML 3.0 CONFIG REQUIRED COMPONENTS Audio)

add_library(meatengine_audio STATIC
    src/audio.cpp
)

target_include_directories(meatengine_audio PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

meatengine_set_target_options(meatengine_audio)

target_link_libraries(meatengine_audio PUBLIC
    meatengine_assets
    SFML::Audio
)
