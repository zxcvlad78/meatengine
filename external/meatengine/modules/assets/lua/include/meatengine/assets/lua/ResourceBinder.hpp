#pragma once

#include <sol/sol.hpp>

#include <meatengine/assets/ResourceLoader.hpp>

namespace me::lua {

    template <typename T>
    void register_resource(sol::table& table, const std::string& name) {
        table.set_function("load_" + name,
            [name](const std::string& path) -> T& {
                auto res = me::ResourceLoader::load<T>(path);
                auto h = res.handle();
                if (!h) throw std::runtime_error("[ResourceLoader] load_" + name + " failed: " + path);
                return *h;
            });

        table.set_function("get_" + name,
            [](const std::string& path) -> T* {
                auto res = me::ResourceLoader::get<T>(path);
                auto h = res.handle();
                return h ? h.get() : nullptr;
            });
    }

} // namespace me::lua
