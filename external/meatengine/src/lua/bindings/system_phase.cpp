#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/SystemPhase.hpp>

static void bind(sol::state& lua) {
    sol::table phases = lua.create_table();

    phases["Input"] = static_cast<int>(me::SystemPhase::Input);
    phases["FixedUpdate"] = static_cast<int>(me::SystemPhase::FixedUpdate);
    phases["Update"] = static_cast<int>(me::SystemPhase::Update);
    phases["Render"] = static_cast<int>(me::SystemPhase::Render);
    phases["PostRender"] = static_cast<int>(me::SystemPhase::PostRender);

    phases["to_string"] = [](int p) { return me::to_string(static_cast<me::SystemPhase>(p)); };

    lua["SystemPhase"] = phases;
}

ME_LUA_BINDING(
    "engine:SystemPhase",
    static_cast<unsigned>(me::lua::BindPriority::BaseType),
    &bind
)
