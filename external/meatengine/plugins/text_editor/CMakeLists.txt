add_library(meatengine_plugin_text_editor STATIC
    src/text_editor.cpp
)

target_include_directories(meatengine_plugin_text_editor PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

meatengine_set_target_options(meatengine_plugin_text_editor)
