#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/cmp/ui/Node.hpp>

static void bind(sol::state& lua) {
    lua.new_enum<me::cmp::ui::Node::PositionType>("PositionType", {
        { "Relative", me::cmp::ui::Node::PositionType::Relative },
        { "Absolute", me::cmp::ui::Node::PositionType::Absolute },
    });

    lua.new_usertype<me::cmp::ui::Node>("Node",
        sol::constructors<me::cmp::ui::Node()>(),
        "width", &me::cmp::ui::Node::width,
        "height", &me::cmp::ui::Node::height,
        "min_width", &me::cmp::ui::Node::min_width,
        "min_height", &me::cmp::ui::Node::min_height,
        "max_width", &me::cmp::ui::Node::max_width,
        "max_height", &me::cmp::ui::Node::max_height,

        "margin", &me::cmp::ui::Node::margin,
        "padding", &me::cmp::ui::Node::padding,
        "border",  &me::cmp::ui::Node::border,

        "position", &me::cmp::ui::Node::position,
        "top", &me::cmp::ui::Node::top,
        "left", &me::cmp::ui::Node::left,
        "right", &me::cmp::ui::Node::right,
        "bottom", &me::cmp::ui::Node::bottom,

        "overflow_clip", &me::cmp::ui::Node::overflow_clip
    );

    me::lua::register_component<me::cmp::ui::Node>("Node", lua);
}

ME_LUA_BINDING(
    "engine:ui:Node",
    static_cast<unsigned>(me::lua::BindPriority::Component) + 10,
    &bind
)