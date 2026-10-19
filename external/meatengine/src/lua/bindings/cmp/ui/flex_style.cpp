#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/cmp/ui/FlexStyle.hpp>

static void bind(sol::state& lua) {
    lua.new_enum<me::cmp::ui::Display>("Display", {
        { "Flex", me::cmp::ui::Display::Flex },
        { "Grid", me::cmp::ui::Display::Grid },
        { "None", me::cmp::ui::Display::None },
    });
    lua.new_enum<me::cmp::ui::FlexDirection>("FlexDirection", {
        { "Row", me::cmp::ui::FlexDirection::Row },
        { "Column", me::cmp::ui::FlexDirection::Column },
        { "RowReverse", me::cmp::ui::FlexDirection::RowReverse },
        { "ColumnReverse", me::cmp::ui::FlexDirection::ColumnReverse },
    });
    lua.new_enum<me::cmp::ui::FlexWrap>("FlexWrap", {
        { "NoWrap", me::cmp::ui::FlexWrap::NoWrap },
        { "Wrap", me::cmp::ui::FlexWrap::Wrap },
        { "WrapReverse", me::cmp::ui::FlexWrap::WrapReverse },
    });
    lua.new_enum<me::cmp::ui::JustifyContent>("JustifyContent", {
        { "FlexStart", me::cmp::ui::JustifyContent::FlexStart },
        { "FlexEnd", me::cmp::ui::JustifyContent::FlexEnd },
        { "Center", me::cmp::ui::JustifyContent::Center },
        { "SpaceBetween", me::cmp::ui::JustifyContent::SpaceBetween },
        { "SpaceAround", me::cmp::ui::JustifyContent::SpaceAround },
        { "SpaceEvenly", me::cmp::ui::JustifyContent::SpaceEvenly },
    });
    lua.new_enum<me::cmp::ui::AlignItems>("AlignItems", {
        { "FlexStart", me::cmp::ui::AlignItems::FlexStart },
        { "FlexEnd", me::cmp::ui::AlignItems::FlexEnd },
        { "Center", me::cmp::ui::AlignItems::Center },
        { "Baseline", me::cmp::ui::AlignItems::Baseline },
        { "Stretch", me::cmp::ui::AlignItems::Stretch },
    });
    lua.new_enum<me::cmp::ui::AlignSelf>("AlignSelf", {
        { "Auto", me::cmp::ui::AlignSelf::Auto },
        { "FlexStart", me::cmp::ui::AlignSelf::FlexStart },
        { "FlexEnd", me::cmp::ui::AlignSelf::FlexEnd },
        { "Center", me::cmp::ui::AlignSelf::Center },
        { "Baseline", me::cmp::ui::AlignSelf::Baseline },
        { "Stretch", me::cmp::ui::AlignSelf::Stretch },
    });
    lua.new_enum<me::cmp::ui::AlignContent>("AlignContent", {
        { "FlexStart", me::cmp::ui::AlignContent::FlexStart },
        { "FlexEnd", me::cmp::ui::AlignContent::FlexEnd },
        { "Center", me::cmp::ui::AlignContent::Center },
        { "SpaceBetween", me::cmp::ui::AlignContent::SpaceBetween },
        { "SpaceAround", me::cmp::ui::AlignContent::SpaceAround },
        { "Stretch", me::cmp::ui::AlignContent::Stretch },
    });

    lua.new_usertype<me::cmp::ui::FlexStyle>("FlexStyle",
        sol::constructors<me::cmp::ui::FlexStyle()>(),
        "display", &me::cmp::ui::FlexStyle::display,
        "direction", &me::cmp::ui::FlexStyle::direction,
        "wrap", &me::cmp::ui::FlexStyle::wrap,
        "justify", &me::cmp::ui::FlexStyle::justify,
        "align_items", &me::cmp::ui::FlexStyle::align_items,
        "align_self", &me::cmp::ui::FlexStyle::align_self,
        "align_content", &me::cmp::ui::FlexStyle::align_content,
        "flex_grow", &me::cmp::ui::FlexStyle::flex_grow,
        "flex_shrink", &me::cmp::ui::FlexStyle::flex_shrink,
        "flex_basis", &me::cmp::ui::FlexStyle::flex_basis,
        "row_gap", &me::cmp::ui::FlexStyle::row_gap,
        "column_gap", &me::cmp::ui::FlexStyle::column_gap
    );

    me::lua::register_component<me::cmp::ui::FlexStyle>("FlexStyle", lua);
}

ME_LUA_BINDING(
    "engine:ui:FlexStyle",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)