#include <godlike/lua_bindings/common.hpp>
#include <godlike/godlike.hpp>

namespace godlike::lua_bindings {
    void init(sol::state& lua) {
        // items

        auto item_definition_ut = lua.new_usertype<godlike::ItemDefinition>("ItemDefinition",
            sol::constructors<godlike::ItemDefinition()>(),
            "id", &godlike::ItemDefinition::id,
            "icon_path", &godlike::ItemDefinition::icon_path,
            "stack_size", &godlike::ItemDefinition::stack_size
        );

        auto item_stack_ut = lua.new_usertype<godlike::ItemStack>("ItemStack",
            sol::constructors<godlike::ItemDefinition()>(),
            "def_idx", &godlike::ItemStack::def_idx,
            "count", &godlike::ItemStack::count
        );

        sol::table ir = lua.create_table();
        lua["ItemRegistry"] = ir;

        ir.set_function("register_def",
            [](const std::string& id, uint32_t max_stack, std::string icon_path) {
                return ItemRegistry::register_def(id, max_stack, std::move(icon_path));
            }
        );

        ir.set_function("get",
            [](uint16_t idx) { return ItemRegistry::get(idx); }
        );

        ir.set_function("find",
            [](const std::string &id) { return ItemRegistry::find(id); }
        );

        ir.set_function("id_of",
            [](uint16_t idx) { return ItemRegistry::id_of(idx); }
        );

        ir.set_function("size",
            []() { return ItemRegistry::size(); }
        );

        // components

        auto move_speed_ut = lua.new_usertype<components::MoveSpeed>("MoveSpeed",
            sol::constructors<components::MoveSpeed()>(),
            "value", &components::MoveSpeed::value
        );

        me::lua_bindings::register_component<components::MoveSpeed>("MoveSpeed", lua);
    }
}