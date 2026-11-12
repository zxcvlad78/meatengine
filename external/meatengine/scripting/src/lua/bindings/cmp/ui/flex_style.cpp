#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/ui/cmp/FlexStyle.hpp>

static void bind(sol::state& lua) {
    lua.new_enum<me::ui::cmp::Display>("Display", {
        { "Flex", me::ui::cmp::Display::Flex },
        { "Grid", me::ui::cmp::Display::Grid },
        { "None", me::ui::cmp::Display::None },
    });
    lua.new_enum<me::ui::cmp::FlexDirection>("FlexDirection", {
        { "Row", me::ui::cmp::FlexDirection::Row },
        { "Column", me::ui::cmp::FlexDirection::Column },
        { "RowReverse", me::ui::cmp::FlexDirection::RowReverse },
        { "ColumnReverse", me::ui::cmp::FlexDirection::ColumnReverse },
    });
    lua.new_enum<me::ui::cmp::FlexWrap>("FlexWrap", {
        { "NoWrap", me::ui::cmp::FlexWrap::NoWrap },
        { "Wrap", me::ui::cmp::FlexWrap::Wrap },
        { "WrapReverse", me::ui::cmp::FlexWrap::WrapReverse },
    });
    lua.new_enum<me::ui::cmp::JustifyContent>("JustifyContent", {
        { "FlexStart", me::ui::cmp::JustifyContent::FlexStart },
        { "FlexEnd", me::ui::cmp::JustifyContent::FlexEnd },
        { "Center", me::ui::cmp::JustifyContent::Center },
        { "SpaceBetween", me::ui::cmp::JustifyContent::SpaceBetween },
        { "SpaceAround", me::ui::cmp::JustifyContent::SpaceAround },
        { "SpaceEvenly", me::ui::cmp::JustifyContent::SpaceEvenly },
    });
    lua.new_enum<me::ui::cmp::AlignItems>("AlignItems", {
        { "FlexStart", me::ui::cmp::AlignItems::FlexStart },
        { "FlexEnd", me::ui::cmp::AlignItems::FlexEnd },
        { "Center", me::ui::cmp::AlignItems::Center },
        { "Baseline", me::ui::cmp::AlignItems::Baseline },
        { "Stretch", me::ui::cmp::AlignItems::Stretch },
    });
    lua.new_enum<me::ui::cmp::AlignSelf>("AlignSelf", {
        { "Auto", me::ui::cmp::AlignSelf::Auto },
        { "FlexStart", me::ui::cmp::AlignSelf::FlexStart },
        { "FlexEnd", me::ui::cmp::AlignSelf::FlexEnd },
        { "Center", me::ui::cmp::AlignSelf::Center },
        { "Baseline", me::ui::cmp::AlignSelf::Baseline },
        { "Stretch", me::ui::cmp::AlignSelf::Stretch },
    });
    lua.new_enum<me::ui::cmp::AlignContent>("AlignContent", {
        { "FlexStart", me::ui::cmp::AlignContent::FlexStart },
        { "FlexEnd", me::ui::cmp::AlignContent::FlexEnd },
        { "Center", me::ui::cmp::AlignContent::Center },
        { "SpaceBetween", me::ui::cmp::AlignContent::SpaceBetween },
        { "SpaceAround", me::ui::cmp::AlignContent::SpaceAround },
        { "Stretch", me::ui::cmp::AlignContent::Stretch },
    });

    lua.new_usertype<me::ui::cmp::FlexStyle>("FlexStyle",
        sol::constructors<me::ui::cmp::FlexStyle()>(),
        "display", &me::ui::cmp::FlexStyle::display,
        "direction", &me::ui::cmp::FlexStyle::direction,
        "wrap", &me::ui::cmp::FlexStyle::wrap,
        "justify", &me::ui::cmp::FlexStyle::justify,
        "align_items", &me::ui::cmp::FlexStyle::align_items,
        "align_self", &me::ui::cmp::FlexStyle::align_self,
        "align_content", &me::ui::cmp::FlexStyle::align_content,
        "flex_grow", &me::ui::cmp::FlexStyle::flex_grow,
        "flex_shrink", &me::ui::cmp::FlexStyle::flex_shrink,
        "flex_basis", &me::ui::cmp::FlexStyle::flex_basis,
        "row_gap", &me::ui::cmp::FlexStyle::row_gap,
        "column_gap", &me::ui::cmp::FlexStyle::column_gap
    );

    me::lua::register_component<me::ui::cmp::FlexStyle>("FlexStyle", lua);
}

ME_LUA_BINDING(
    "engine:ui:FlexStyle",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)