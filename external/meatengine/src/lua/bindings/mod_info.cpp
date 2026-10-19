#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/ModInfo.hpp>

static void bind(sol::state& lua) {
    lua.new_enum<me::ModDependencyType>("ModDependencyType", {
        { "Required", me::ModDependencyType::Required },
        { "Optional", me::ModDependencyType::Optional },
        { "Conflict", me::ModDependencyType::Conflict },
        { "LoadBefore", me::ModDependencyType::LoadBefore },
    });

    lua.new_usertype<me::ModVersion>("ModVersion",
        sol::no_constructor,

        "major", &me::ModVersion::major,
        "minor", &me::ModVersion::minor,
        "patch", &me::ModVersion::patch,
        "parse", [](const std::string& s) { return me::ModVersion::parse(s); },
        "to_string", &me::ModVersion::to_string,
        sol::meta_function::to_string, &me::ModVersion::to_string,
        sol::meta_function::less_than, &me::ModVersion::operator<,
        sol::meta_function::equal_to, [](const me::ModVersion& a, const me::ModVersion& b) { return a == b; },
        sol::meta_function::less_than_or_equal_to, &me::ModVersion::operator<=
    );

    lua.new_usertype<me::ModDependency>("ModDependency",
        sol::no_constructor,

        "type", &me::ModDependency::type,
        "name", &me::ModDependency::name,
        "has_version", &me::ModDependency::has_version,
        "op", &me::ModDependency::op,
        "version", &me::ModDependency::version,
        "parse", [](const std::string& s) { return me::ModDependency::parse(s); }
    );

    lua.new_usertype<me::ModInfo>("ModInfo",
        sol::no_constructor,

        "name", &me::ModInfo::name,
        "version", &me::ModInfo::version,
        "path", &me::ModInfo::path,
        "dependencies", &me::ModInfo::dependencies
    );
}

ME_LUA_BINDING(
    "engine:ModInfo",
    static_cast<unsigned>(me::lua::BindPriority::BaseType),
    &bind
)