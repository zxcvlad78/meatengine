#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/ui/cmp/FocusPolicy.hpp>

static void bind(sol::state& lua) {
    lua.new_enum<me::ui::cmp::FocusPolicy::Kind>("FocusPolicyKind", {
        { "None",  me::ui::cmp::FocusPolicy::Kind::None },
        { "Block", me::ui::cmp::FocusPolicy::Kind::Block },
        { "Pass",  me::ui::cmp::FocusPolicy::Kind::Pass },
    });

    lua.new_usertype<me::ui::cmp::FocusPolicy>("FocusPolicy",
        sol::constructors<me::ui::cmp::FocusPolicy()>(),
        "kind", &me::ui::cmp::FocusPolicy::kind
    );

    me::lua::register_component<me::ui::cmp::FocusPolicy>("FocusPolicy", lua);
}

ME_LUA_BINDING(
    "engine:ui:FocusPolicy",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)