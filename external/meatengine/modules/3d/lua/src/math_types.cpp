#include <string>

#include <sol/sol.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <meatengine/scripting/LuaBinder.hpp>

// Lua types for 3D math: Vec3 (glm::vec3) and Quat (glm::quat).

static void bind(sol::state& lua) {
    {
        auto ut = lua.new_usertype<glm::vec3>("Vec3",
            sol::constructors<glm::vec3(), glm::vec3(float), glm::vec3(float, float, float)>(),
            "x", &glm::vec3::x,
            "y", &glm::vec3::y,
            "z", &glm::vec3::z
        );

        ut[sol::meta_function::addition] = [](const glm::vec3& a, const glm::vec3& b) { return a + b; };
        ut[sol::meta_function::subtraction] = [](const glm::vec3& a, const glm::vec3& b) { return a - b; };
        ut[sol::meta_function::multiplication] = sol::overload(
            [](const glm::vec3& a, const glm::vec3& b) { return a * b; },
            [](const glm::vec3& a, float s) { return a * s; },
            [](float s, const glm::vec3& a) { return a * s; }
        );
        ut[sol::meta_function::division] = sol::overload(
            [](const glm::vec3& a, const glm::vec3& b) { return a / b; },
            [](const glm::vec3& a, float s) { return a / s; }
        );
        ut[sol::meta_function::unary_minus] = [](const glm::vec3& a) { return -a; };
        ut[sol::meta_function::equal_to] = [](const glm::vec3& a, const glm::vec3& b) { return a == b; };
        ut[sol::meta_function::to_string] = [](const glm::vec3& v) {
            return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
        };
    }

    {
        auto ut = lua.new_usertype<glm::quat>("Quat",
            sol::constructors<glm::quat(), glm::quat(float, float, float, float)>(),
            "w", &glm::quat::w,
            "x", &glm::quat::x,
            "y", &glm::quat::y,
            "z", &glm::quat::z
        );

        ut[sol::meta_function::multiplication] = sol::overload(
            [](const glm::quat& a, const glm::quat& b) { return a * b; },
            [](const glm::quat& q, const glm::vec3& v) { return q * v; }
        );
        ut[sol::meta_function::to_string] = [](const glm::quat& q) {
            return "(" + std::to_string(q.w) + ", " + std::to_string(q.x) + ", "
                       + std::to_string(q.y) + ", " + std::to_string(q.z) + ")";
        };
    }
}

ME_LUA_BINDING(
    "engine:3d:math",
    static_cast<unsigned>(me::lua::BindPriority::BaseType),
    &bind
)
