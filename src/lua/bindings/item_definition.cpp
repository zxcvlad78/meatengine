#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <godlike/ItemDefinition.hpp>

static void bind(sol::state& lua) {
    auto usertype = lua.new_usertype<godlike::ItemDefinition>("ItemDefinition",
        sol::constructors<godlike::ItemDefinition()>(),
        "id", &godlike::ItemDefinition::id,
        "icon_path", &godlike::ItemDefinition::icon_path,
        "stack_size", &godlike::ItemDefinition::stack_size
    );
}

ME_LUA_BINDING(
    "godlike:ItemDefinition",
    static_cast<unsigned>(me::lua::BindPriority::BaseType),
    &bind
)
