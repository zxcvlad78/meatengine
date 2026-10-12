#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <godlike/ItemRegistry.hpp>

static void bind(sol::state& lua) {
    sol::table table = lua.create_table();
    lua["ItemRegistry"] = table;

    table.set_function("register",
        [](const std::string& id, uint32_t max_stack, std::string icon_path) {
            return godlike::ItemRegistry::register_def(id, max_stack, std::move(icon_path));
        }
    );

    table.set_function("get",
        [](uint16_t idx) { return godlike::ItemRegistry::get(idx); }
    );

    table.set_function("find",
        [](const std::string &id) { return godlike::ItemRegistry::find(id); }
    );

    table.set_function("id_of",
        [](uint16_t idx) { return godlike::ItemRegistry::id_of(idx); }
    );

    table.set_function("size",
        []() { return godlike::ItemRegistry::size(); }
    );
}

ME_LUA_BINDING(
    "godlike:api:ItemRegistry",
    static_cast<unsigned>(me::lua::BindPriority::API),
    &bind
)
