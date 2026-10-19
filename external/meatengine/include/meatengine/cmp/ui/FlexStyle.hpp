#pragma once
#include <meatengine/cmp/ui/Val.hpp>
#include <cstdint>

namespace me::cmp::ui {

    enum class Display : uint8_t { Flex, Grid, None };
    enum class FlexDirection : uint8_t { Row, Column, RowReverse, ColumnReverse };
    enum class FlexWrap : uint8_t { NoWrap, Wrap, WrapReverse };
    enum class JustifyContent : uint8_t { FlexStart, FlexEnd, Center, SpaceBetween, SpaceAround, SpaceEvenly };
    enum class AlignItems : uint8_t { FlexStart, FlexEnd, Center, Baseline, Stretch };
    enum class AlignSelf : uint8_t { Auto, FlexStart, FlexEnd, Center, Baseline, Stretch };
    enum class AlignContent : uint8_t { FlexStart, FlexEnd, Center, SpaceBetween, SpaceAround, Stretch };

    struct FlexStyle {
        Display display = Display::Flex;
        FlexDirection direction = FlexDirection::Row;
        FlexWrap wrap = FlexWrap::NoWrap;
        JustifyContent justify = JustifyContent::FlexStart;
        AlignItems align_items = AlignItems::Stretch;
        AlignSelf align_self = AlignSelf::Auto;
        AlignContent align_content = AlignContent::FlexStart;

        float flex_grow = 0.f;
        float flex_shrink = 1.f;
        Val flex_basis = Val::Auto();

        float row_gap = 0.f;
        float column_gap = 0.f;
    };

}