#pragma once
#include <meatengine/cmp/ui/Val.hpp>
#include <meatengine/cmp/ui/UiRect.hpp>
#include <cstdint>

namespace me::cmp::ui {

    struct Node {
        enum class PositionType : uint8_t { Relative, Absolute };

        Val width = Val::Auto();
        Val height = Val::Auto();
        Val min_width = Val::Auto();
        Val min_height = Val::Auto();
        Val max_width = Val::Auto();
        Val max_height = Val::Auto();

        UiRect margin = UiRect::all(Val::Px(0.f));
        UiRect padding = UiRect::all(Val::Px(0.f));
        UiRect border = UiRect::all(Val::Px(0.f));

        PositionType position = PositionType::Relative;
        Val top = Val::Auto();
        Val left = Val::Auto();
        Val right = Val::Auto();
        Val bottom = Val::Auto();

        bool overflow_clip = false;
    };

}