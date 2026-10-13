#pragma once
#include <meatengine/cmp/ui/Val.hpp>

namespace me::cmp::ui {

    struct UiRect {
        Val top = Val::Auto();
        Val right = Val::Auto();
        Val bottom = Val::Auto();
        Val left = Val::Auto();

        static UiRect all(Val v) { return {v, v, v, v}; }
        static UiRect axes(Val vert, Val horiz) { return {vert, horiz, vert, horiz}; }
    };

}