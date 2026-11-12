#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/ui/cmp/Node.hpp>

static void bind(sol::state& lua) {
    lua.new_enum<me::ui::cmp::Node::PositionType>("PositionType", {
        { "Relative", me::ui::cmp::Node::PositionType::Relative },
        { "Absolute", me::ui::cmp::Node::PositionType::Absolute },
    });

    lua.new_usertype<me::ui::cmp::Node>("Node",
        sol::constructors<me::ui::cmp::Node()>(),
        "width", &me::ui::cmp::Node::width,
        "height", &me::ui::cmp::Node::height,
        "min_width", &me::ui::cmp::Node::min_width,
        "min_height", &me::ui::cmp::Node::min_height,
        "max_width", &me::ui::cmp::Node::max_width,
        "max_height", &me::ui::cmp::Node::max_height,

        "margin", &me::ui::cmp::Node::margin,
        "padding", &me::ui::cmp::Node::padding,
        "border",  &me::ui::cmp::Node::border,

        "position", &me::ui::cmp::Node::position,
        "top", &me::ui::cmp::Node::top,
        "left", &me::ui::cmp::Node::left,
        "right", &me::ui::cmp::Node::right,
        "bottom", &me::ui::cmp::Node::bottom,

        "overflow_clip", &me::ui::cmp::Node::overflow_clip
    );

    me::lua::register_component<me::ui::cmp::Node>("Node", lua);
}

ME_LUA_BINDING(
    "engine:ui:Node",
    static_cast<unsigned>(me::lua::BindPriority::Component) + 10,
    &bind
)