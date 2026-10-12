#include <string>
#include <meatengine/lua/LuaBinder.hpp>

static void bind(sol::state& lua) {
    //struct BindingEntry { std::string name; unsigned int priority = 0; std::function<void(sol::state&)> func; };

    lua.new_usertype<me::lua::BindingEntry>("LuaBindingEntry",
        sol::no_constructor,
        "name", &me::lua::BindingEntry::name,
        "priority", &me::lua::BindingEntry::priority
    );

    sol::table table = lua.create_table();
    lua["LuaBinder"] = table;

    table.set_function("list", []() {
        return me::lua::binding_list();
    });
    
}

ME_LUA_BINDING(
    "engine:api:LuaBinder",
    static_cast<unsigned>(me::lua::BindPriority::API),
    &bind
)