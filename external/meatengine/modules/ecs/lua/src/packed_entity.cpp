#include <string>
#include <iostream>
#include <meatengine/scripting/LuaBinder.hpp>

#include <meatengine/ecs/PackedEntity.hpp>

static void bind(sol::state& lua) {
    sol::table table = lua.create_table();
    lua["PackedEntity"] = table;

    table.set_function("register",
        [](const std::string& id, sol::protected_function fn) {
            me::PackedEntity::register_pe(id, [id, fn](entt::registry& reg) -> entt::entity {
                sol::protected_function_result r = fn(reg);
                if (!r.valid()) {
                    sol::error err = r;
                    std::cerr << "[PackedEntity] '" << id << "' failed: " << err.what() << "\n";
                    return entt::null;
                }

                if (r.get_type() == sol::type::nil) {
                    std::cerr << "[PackedEntity] '" << id << "' returned nil\n";
                    return entt::null;
                }

                try {
                    return r.get<entt::entity>();
                } catch (const std::exception& e) {
                    std::cerr << "[PackedEntity] '" << id << "' bad return: " << e.what() << "\n";
                    return entt::null;
                }
            });
        });

    table.set_function("spawn",
        [](const std::string& id, entt::registry& reg) -> entt::entity {
            return me::PackedEntity::spawn(id, reg);
        });

    table.set_function("exists",
        [](const std::string& id) -> bool {
            return me::PackedEntity::exists(id);
        });
}

ME_LUA_BINDING(
    "engine:api:PackedEntity",
    static_cast<unsigned>(me::lua::BindPriority::API),
    &bind
)
