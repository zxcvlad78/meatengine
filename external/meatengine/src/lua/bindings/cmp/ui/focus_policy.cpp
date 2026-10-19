#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/cmp/ui/FocusPolicy.hpp>

static void bind(sol::state& lua) {
    lua.new_enum<me::cmp::ui::FocusPolicy::Kind>("FocusPolicyKind", {
        { "None",  me::cmp::ui::FocusPolicy::Kind::None },
        { "Block", me::cmp::ui::FocusPolicy::Kind::Block },
        { "Pass",  me::cmp::ui::FocusPolicy::Kind::Pass },
    });

    lua.new_usertype<me::cmp::ui::FocusPolicy>("FocusPolicy",
        sol::constructors<me::cmp::ui::FocusPolicy()>(),
        "kind", &me::cmp::ui::FocusPolicy::kind
    );

    me::lua::register_component<me::cmp::ui::FocusPolicy>("FocusPolicy", lua);
}

ME_LUA_BINDING(
    "engine:ui:FocusPolicy",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)