#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/cmp/ui/ComputedNode.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::cmp::ui::ComputedNode>("ComputedNode",
        sol::no_constructor,
        "size", &me::cmp::ui::ComputedNode::size,
        "content_size", &me::cmp::ui::ComputedNode::content_size,
        "stack_index", &me::cmp::ui::ComputedNode::stack_index
    );

    me::lua::register_component<me::cmp::ui::ComputedNode>("ComputedNode", lua);
}

ME_LUA_BINDING(
    "engine:ui:ComputedNode",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)