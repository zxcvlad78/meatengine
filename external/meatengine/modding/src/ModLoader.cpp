#include <meatengine/ModLoader.hpp>
#include <meatengine/ScriptingServer.hpp>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <system_error>

#include <sol/sol.hpp>

namespace fs = std::filesystem;

namespace me {

namespace {
    bool version_satisfies(const ModVersion& v, const std::string& op, const ModVersion& want) {
        if (op == "==") return v == want;
        if (op == ">=") return !(v < want);
        if (op == "<=") return v < want || v == want;
        if (op == ">")  return want < v;
        if (op == "<")  return v < want;
        throw std::runtime_error("[ModLoader] unknown version operator '" + op + "'");
    }
}

std::vector<ModInfo> ModLoader::list() {
    std::vector<ModInfo> out;
    out.reserve(_load_order.size());
    for (const auto& name : _load_order) {
        auto it = _name_map.find(name);
        if (it != _name_map.end())
            out.push_back(it->second);
    }
    return out;
}

const ModInfo* ModLoader::find(const std::string& name) {
    auto it = _name_map.find(name);
    return it == _name_map.end() ? nullptr : &it->second;
}

const ModInfo& ModLoader::get(const std::string& name) {
    auto it = _name_map.find(name);
    if (it == _name_map.end())
        throw std::out_of_range("[ModLoader] mod not loaded: '" + name + "'");
    return it->second;
}

bool ModLoader::is_loaded(const ModInfo& mi) { return is_loaded(mi.name); }
bool ModLoader::is_loaded(const std::string& name) { return _name_map.find(name) != _name_map.end(); }

bool ModLoader::load(const std::string& dir) {
    if (dir.empty()) {
        std::cerr << "[ModLoader] load: empty dir\n";
        return false;
    }

    std::error_code ec;
    if (!fs::exists(dir, ec) || ec) {
        std::cerr << "[ModLoader] load: path does not exist: " << dir << "\n";
        return false;
    }
    if (!fs::is_directory(dir, ec) || ec) {
        std::cerr << "[ModLoader] load: not a directory: " << dir << "\n";
        return false;
    }

    std::vector<ModInfo> mods;
    try {
        mods = scan_mods(dir);
    } catch (const std::exception& e) {
        std::cerr << "[ModLoader] scan failed: " << e.what() << "\n";
        return false;
    }

    std::vector<std::size_t> order;
    try {
        order = resolve_order(mods);
    } catch (const std::exception& e) {
        std::cerr << "[ModLoader] resolve failed: " << e.what() << "\n";
        return false;
    }

    _name_map.clear();
    _load_order.clear();
    _load_dir = dir;

    for (std::size_t idx : order)
        load_mod(mods[idx]);

    return true;
}

bool ModLoader::load() {
    const std::string dir = _load_dir.empty() ? std::string{"data"} : _load_dir;
    return load(dir);
}

bool ModLoader::load_mod(const ModInfo& mod) {
    fs::path init_path = fs::path(mod.path) / "init.lua";
    std::error_code ec;
    if (!fs::exists(init_path, ec) || ec) {
        std::cerr << "[ModLoader] missing init.lua in '" << mod.name << "'\n";
        return false;
    }

    auto& lua = ScriptingServer::lua();
    lua["MOD_NAME"] = mod.name;
    lua["MOD_PATH"] = mod.path;

    bool ok = ScriptingServer::run_file(init_path.string());

    lua["MOD_NAME"] = "";
    lua["MOD_PATH"] = "";

    if (!ok) {
        std::cerr << "[ModLoader] failed to load '" << mod.name << "'\n";
        return false;
    }

    auto [it, inserted] = _name_map.emplace(mod.name, mod);
    if (inserted)
        _load_order.push_back(mod.name);

    std::cout << "[ModLoader] successfully loaded '" << mod.name << "'\n";
    return true;
}

std::vector<ModInfo> ModLoader::scan_mods(const std::string& path) {
    std::vector<ModInfo> result;

    std::error_code ec;
    fs::directory_iterator it(path, ec);
    if (ec) {
        throw std::runtime_error(
            "[ModLoader] cannot iterate '" + path + "': " + ec.message());
    }

    for (const auto& entry : it) {
        if (!entry.is_directory()) continue;

        ModInfo info;
        info.path = entry.path().string();
        if (!read_mod_info(info.path, info))
            continue;

        result.push_back(std::move(info));
    }

    std::sort(result.begin(), result.end(),
        [](const ModInfo& a, const ModInfo& b) { return a.name < b.name; });

    return result;
}

bool ModLoader::read_mod_info(const std::string& path, ModInfo& out) {
    fs::path infoPath = fs::path(path) / "modinfo.lua";

    std::error_code ec;
    if (!fs::exists(infoPath, ec) || ec)
        return false;

    auto& lua = ScriptingServer::lua();

    sol::protected_function_result r = lua.safe_script_file(infoPath.string());
    if (!r.valid()) {
        sol::error err = r;
        std::cerr << "[ModLoader] bad modinfo.lua in '" << path << "': " << err.what() << "\n";
        return false;
    }

    sol::object obj = r;
    if (!obj.is<sol::table>()) {
        std::cerr << "[ModLoader] modinfo.lua in '" << path << "' did not return a table\n";
        return false;
    }
    sol::table t = obj.as<sol::table>();

    out.name = t.get_or("name", std::string{});
    if (out.name.empty()) {
        std::cerr << "[ModLoader] modinfo.lua in '" << path << "' has no 'name'\n";
        return false;
    }

    out.title = t.get_or("title", std::string{});
    out.description = t.get_or("description", std::string{});

    std::string ver = t.get_or("version", std::string{"0.0.0"});
    try {
        out.version = ModVersion::parse(ver);
    } catch (const std::exception& e) {
        std::cerr << "[ModLoader] mod '" << out.name << "': bad version '" << ver << "': " << e.what() << "\n";
        return false;
    }

    if (auto deps = t.get<sol::optional<sol::table>>("dependencies"); deps.has_value()) {
        for (auto&& [_, v] : *deps) {
            if (!v.is<std::string>()) continue;
            try {
                out.dependencies.push_back(ModDependency::parse(v.as<std::string>()));
            } catch (const std::exception& e) {
                std::cerr << "[ModLoader] mod '" << out.name
                          << "': bad dependency '" << v.as<std::string>()
                          << "': " << e.what() << "\n";
            }
        }
    }
    return true;
}

std::vector<std::size_t> ModLoader::resolve_order(const std::vector<ModInfo>& mods) {
    const std::size_t n = mods.size();

    std::unordered_map<std::string, std::size_t> byName;
    for (std::size_t i = 0; i < n; ++i) {
        if (!byName.emplace(mods[i].name, i).second)
            throw std::runtime_error(
                "[ModLoader] duplicate mod name '" + mods[i].name + "'");
    }

    std::vector<std::vector<std::size_t>> edges(n);
    std::vector<std::size_t> indegree(n, 0);

    for (std::size_t i = 0; i < n; ++i) {
        for (const auto& dep : mods[i].dependencies) {
            auto it = byName.find(dep.name);
            bool present = (it != byName.end());

            switch (dep.type) {
                case ModDependencyType::Conflict:
                    if (present)
                        throw std::runtime_error(
                            "[ModLoader] '" + mods[i].name + "' conflicts with '" + dep.name + "'"
                        );
                    break;

                case ModDependencyType::Required: {
                    if (!present)
                        throw std::runtime_error(
                            "[ModLoader] '" + mods[i].name + "' requires missing mod '" + dep.name + "'"
                        );
                    std::size_t j = it->second;
                    if (dep.has_version
                        && !version_satisfies(mods[j].version, dep.op, dep.version))
                        throw std::runtime_error(
                            "[ModLoader] '" + mods[i].name + "' requires " + dep.name
                            + " " + dep.op + " " + dep.version.to_string()
                            + ", but got " + mods[j].version.to_string());
                    edges[j].push_back(i);
                    ++indegree[i];
                    break;
                }

                case ModDependencyType::Optional: {
                    if (present) {
                        std::size_t j = it->second;
                        edges[j].push_back(i);
                        ++indegree[i];
                    }
                    break;
                }

                case ModDependencyType::LoadBefore: {
                    if (present) {
                        std::size_t j = it->second;
                        edges[i].push_back(j);
                        ++indegree[j];
                    }
                    break;
                }
            }
        }
    }

    std::vector<std::size_t> order;
    order.reserve(n);

    std::vector<std::size_t> ready;
    for (std::size_t i = 0; i < n; ++i)
        if (indegree[i] == 0) ready.push_back(i);

    while (!ready.empty()) {
        auto it = std::min_element(ready.begin(), ready.end());
        std::size_t v = *it;
        ready.erase(it);
        order.push_back(v);

        for (std::size_t w : edges[v]) {
            if (--indegree[w] == 0)
                ready.push_back(w);
        }
    }

    if (order.size() != n) {
        std::string cycle;
        for (std::size_t i = 0; i < n; ++i) {
            if (indegree[i] > 0) {
                if (!cycle.empty()) cycle += ", ";
                cycle += mods[i].name;
            }
        }
        throw std::runtime_error("[ModLoader] dependency cycle among: " + cycle);
    }

    return order;
}

}