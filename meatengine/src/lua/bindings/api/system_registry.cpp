#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/SystemRegistry.hpp>

static void bind(sol::state& lua) {
    auto table = lua.create_table();
    lua["SystemRegistry"] = table;

    auto emplace_fn = [](
        me::SystemPhase phase,
        const std::string& id,
        int priority,
        sol::protected_function fn
    ) {
        if (phase == me::SystemPhase::Invalid) {
            throw std::runtime_error("[SystemRegistry] invalid phase for '" + id + "'");
        }
        if (!fn.valid()) {
            throw std::runtime_error("[SystemRegistry] invalid function for '" + id + "'");
        }

        me::SystemRegistry::emplace(phase, id, priority,
            [id, fn]() mutable {
                auto r = fn();
                if (!r.valid()) {
                    sol::error err = r;
                    std::cerr << "[SystemRegistry] emplace '" << id << "': " << err.what() << "\n";
                }
            });
    };

    table.set_function("emplace",
    [](me::SystemPhase phase,
       const std::string& id,
       int priority,
       sol::protected_function fn
    ) {
        if (!fn.valid()) {
            throw std::runtime_error("[SystemRegistry] invalid function for '" + id + "'");
        }

        me::SystemRegistry::emplace(phase, id, priority,
            [id, fn]() mutable {
                sol::protected_function_result r = fn();

                if (!r.valid()) {
                    sol::error err = r;
                    std::cerr << "[lua system '" << id << "'] " << err.what() << "\n";
                }
            });
    });

    table.set_function("emplace", sol::overload(
        [emplace_fn](int phase, const std::string& id, int priority, sol::protected_function fn) {
            emplace_fn(static_cast<me::SystemPhase>(phase), id, priority, std::move(fn));
        },
        [table, emplace_fn](const std::string& phase, const std::string& id, int priority, sol::protected_function fn) {
            auto p = me::from_string(phase);
            if (p == me::SystemPhase::Invalid) {
                throw std::runtime_error("[SystemRegistry] unknown phase '" + phase + "'");
            }
            emplace_fn(p, id, priority, std::move(fn));
        }
    ));

    table.set_function("remove", sol::overload(
        [](const std::string& id) { me::SystemRegistry::remove(id); },
        [](const std::string& id, int phase) {
            me::SystemRegistry::remove(id, static_cast<me::SystemPhase>(phase));
        },
        [](const std::string& id, const std::string& phase) {
            me::SystemRegistry::remove(id, me::from_string(phase));
        }
    ));

    table.set_function("clear", sol::overload(
        []() { me::SystemRegistry::clear(); },
        [](int phase) { me::SystemRegistry::clear(static_cast<me::SystemPhase>(phase)); },
        [](const std::string& phase) { me::SystemRegistry::clear(me::from_string(phase)); }
    ));

    table.set_function("has", sol::overload(
        [](const std::string& id) { return me::SystemRegistry::has(id); },
        [](const std::string& id, int phase) {
            return me::SystemRegistry::has(id, static_cast<me::SystemPhase>(phase));
        },
        [](const std::string& id, const std::string& phase) {
            return me::SystemRegistry::has(id, me::from_string(phase));
        }
    ));

    table.set_function("list", sol::overload(
        []() { return me::SystemRegistry::list(); },
        [](int phase) { return me::SystemRegistry::list(static_cast<me::SystemPhase>(phase)); },
        [](const std::string& phase) { return me::SystemRegistry::list(me::from_string(phase)); }
    ));

    table.set_function("clear", sol::overload(
        []() { me::SystemRegistry::clear(); },
        [](int phase) { me::SystemRegistry::clear(static_cast<me::SystemPhase>(phase)); },
        [](const std::string& phase) { me::SystemRegistry::clear(me::from_string(phase)); }
    ));

    table.set_function("run", sol::overload(
        [](int phase) { me::SystemRegistry::run(static_cast<me::SystemPhase>(phase)); },
        [](const std::string& phase) {
            auto p = me::from_string(phase);
            if (p == me::SystemPhase::Invalid)
                throw std::runtime_error("[SystemRegistry] unknown phase '" + phase + "'");
            me::SystemRegistry::run(p);
        }
    ));
}

ME_LUA_BINDING(
    "engine:api:SystemRegistry",
    static_cast<unsigned>(me::lua::BindPriority::API),
    &bind
)