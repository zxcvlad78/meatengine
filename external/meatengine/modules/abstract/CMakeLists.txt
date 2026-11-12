add_library(meatengine_abstract STATIC
    src/abstract.cpp
)

target_include_directories(meatengine_abstract PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

meatengine_set_target_options(meatengine_abstract)

if(MEATENGINE_WITH_SCRIPTING)
    add_subdirectory(lua)
endif()
