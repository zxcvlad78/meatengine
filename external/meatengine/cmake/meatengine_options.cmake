function(meatengine_set_target_options target)
    target_compile_features(${target} PUBLIC cxx_std_20)

    if(MSVC)
        target_compile_options(${target} PUBLIC
            /W4
            /permissive-
            /Zc:__cplusplus
            /Zc:preprocessor
            /utf-8
            /MP
        )
        target_compile_definitions(${target} PUBLIC
            NOMINMAX
            WIN32_LEAN_AND_MEAN
            _CRT_SECURE_NO_WARNINGS
        )
    else()
        target_compile_options(${target} PUBLIC
            -Wall
            -Wextra
            -Wpedantic
            -Wno-gnu-anonymous-struct
            -Wno-nested-anon-types
        )
    endif()
endfunction()
