#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/ui/cmp/ComputedNode.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::ui::cmp::ComputedNode>("ComputedNode",
        sol::no_constructor,
        "size", &me::ui::cmp::ComputedNode::size,
        "content_size", &me::ui::cmp::ComputedNode::content_size,
        "stack_index", &me::ui::cmp::ComputedNode::stack_index
    );

    me::lua::register_component<me::ui::cmp::ComputedNode>("ComputedNode", lua);
}

ME_LUA_BINDING(
    "engine:ui:ComputedNode",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)