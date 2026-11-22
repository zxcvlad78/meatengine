#include <sol/sol.hpp>

#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/td/cmp/DirectionalLight.hpp>
#include <meatengine/td/cmp/PointLight.hpp>
#include <meatengine/td/cmp/SpotLight.hpp>

// Lua bindings for the three light components.

static void bind(sol::state& lua) {
    lua.new_usertype<me::td::cmp::DirectionalLight>("DirectionalLight",
        sol::constructors<me::td::cmp::DirectionalLight()>(),
        "color", &me::td::cmp::DirectionalLight::color,
        "intensity", &me::td::cmp::DirectionalLight::intensity,
        "direction", &me::td::cmp::DirectionalLight::direction
    );
    me::lua::register_component<me::td::cmp::DirectionalLight>("DirectionalLight", lua);

    lua.new_usertype<me::td::cmp::PointLight>("PointLight",
        sol::constructors<me::td::cmp::PointLight()>(),
        "color", &me::td::cmp::PointLight::color,
        "intensity", &me::td::cmp::PointLight::intensity,
        "range", &me::td::cmp::PointLight::range
    );
    me::lua::register_component<me::td::cmp::PointLight>("PointLight", lua);

    lua.new_usertype<me::td::cmp::SpotLight>("SpotLight",
        sol::constructors<me::td::cmp::SpotLight()>(),
        "color", &me::td::cmp::SpotLight::color,
        "intensity", &me::td::cmp::SpotLight::intensity,
        "range", &me::td::cmp::SpotLight::range,
        "inner_angle", &me::td::cmp::SpotLight::inner_angle_deg,
        "outer_angle", &me::td::cmp::SpotLight::outer_angle_deg,
        "direction", &me::td::cmp::SpotLight::direction
    );
    me::lua::register_component<me::td::cmp::SpotLight>("SpotLight", lua);
}

ME_LUA_BINDING(
    "engine:3d:cmp:lights",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)
