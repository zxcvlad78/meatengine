#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <godlike/ItemStack.hpp>

static void bind(sol::state& lua) {
    auto item_stack_ut = lua.new_usertype<godlike::ItemStack>("ItemStack",
        sol::constructors<godlike::ItemStack()>(),
        "def_idx", &godlike::ItemStack::def_idx,
        "count", &godlike::ItemStack::count
    );
}

ME_LUA_BINDING(
    "godlike:ItemStack",
    static_cast<unsigned>(me::lua::BindPriority::BaseType),
    &bind
)
