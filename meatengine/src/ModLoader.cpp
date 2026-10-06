#include <meatengine/ModLoader.hpp>
#include <meatengine/ScriptingServer.hpp>
#include <meatengine/FileSystem.hpp>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <unordered_set>
#include <nlohmann/json.hpp>
#include <sol/sol.hpp>


namespace fs = std::filesystem;
using json = nlohmann::json;

namespace me {

namespace {
    bool version_satisfies(const modinfo::Version& v, const std::string& op, const modinfo::Version& want) {
        if (op == "==") return v == want;
        if (op == ">=") return !(v < want);
        if (op == "<=") return v < want || v == want;
        if (op == ">")  return want < v;
        if (op == "<")  return v < want;
        throw std::runtime_error("[ModLoader] unknown version operator '" + op + "'");
    }
}

bool ModLoader::init_dir(const std::string& path) {
    if (!fs::exists(path) || !fs::is_directory(path))
        return false;

    std::vector<modinfo::ModInfo> mods;
    try {
        mods = scan_mods(path);
    } catch (const std::exception&) {
        return false;
    }

    std::vector<std::size_t> order;
    try {
        order = resolve_order(mods);
    } catch (const std::exception&) {
        return false;
    }

    for (std::size_t idx : order)
        load_mod(mods[idx]);

    return true;
}

std::vector<modinfo::ModInfo> ModLoader::scan_mods(const std::string& path) {
    std::vector<modinfo::ModInfo> result;

    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(path, ec)) {
        if (ec) break;
        if (!entry.is_directory()) continue;

        modinfo::ModInfo info;
        info.path = entry.path().string();
        if (!read_mod_info(info.path, info))
            continue;

        result.push_back(std::move(info));
    }

    std::sort(
        result.begin(),
        result.end(),
        [](const modinfo::ModInfo& a, const modinfo::ModInfo& b) { return a.name < b.name; }
    );
    return result;
}

bool ModLoader::read_mod_info(const std::string& path, modinfo::ModInfo& out) {
    fs::path infoPath = fs::path(path) / "modinfo.lua";
    if (!fs::exists(infoPath)) return false;

    auto& lua = ScriptingServer::lua();

    sol::protected_function_result r = lua.safe_script_file(infoPath.string());
    if (!r.valid()) return false;

    sol::object obj = r;
    if (!obj.is<sol::table>()) return false;
    sol::table t = obj.as<sol::table>();

    out.name = t.get_or("name", std::string{});
    if (out.name.empty()) return false;

    std::string ver = t.get_or("version", std::string{"0.0.0"});
    try {
        out.version = modinfo::Version::parse(ver);
    } catch (...) {
        return false;
    }

    sol::object depsObj = t["dependencies"];
    if (depsObj.valid() && depsObj.is<sol::table>()) {
        sol::table deps = depsObj.as<sol::table>();
        for (auto&& [_, v] : deps) {
            if (!v.is<std::string>()) continue;
            try {
                out.dependencies.push_back(modinfo::Dependency::parse(v.as<std::string>()));
            } catch (...) { }
        }
    }
    return true;
}

std::vector<std::size_t> ModLoader::resolve_order(const std::vector<modinfo::ModInfo>& mods) {
    const std::size_t n = mods.size();

    std::unordered_map<std::string, std::size_t> byName;
    for (std::size_t i = 0; i < n; ++i) {
        if (!byName.emplace(mods[i].name, i).second)
            throw std::runtime_error("[ModLoader] duplicate mod name '" + mods[i].name + "'");
    }

    std::vector<std::vector<std::size_t>> edges(n);
    std::vector<std::size_t> indegree(n, 0);

    for (std::size_t i = 0; i < n; ++i) {
        for (const auto& dep : mods[i].dependencies) {
            auto it = byName.find(dep.name);
            bool present = (it != byName.end());
            std::size_t j = present ? it->second : 0;

            switch (dep.type) {
                case modinfo::DependencyType::Conflict:
                    if (present)
                        throw std::runtime_error(
                            "[ModLoader] '" + mods[i].name + "' conflicts with '" + dep.name + "'");
                    break;

                case modinfo::DependencyType::Required:
                    if (!present)
                        throw std::runtime_error(
                            "[ModLoader] '" + mods[i].name + "' requires missing mod '" + dep.name + "'");
                    if (dep.has_version && !version_satisfies(mods[j].version, dep.op, dep.version))
                        throw std::runtime_error(
                            "[ModLoader] '" + mods[i].name + "' requires " + dep.name +
                            " " + dep.op + " " + dep.version.to_string() +
                            ", but got " + mods[j].version.to_string());
                    edges[j].push_back(i);
                    ++indegree[i];
                    break;

                case modinfo::DependencyType::Optional:
                    if (present) {
                        edges[j].push_back(i);
                        ++indegree[i];
                    }
                    break;

                case modinfo::DependencyType::LoadBefore:
                    if (present) {
                        edges[i].push_back(j);
                        ++indegree[j];
                    }
                    break;
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

bool ModLoader::load_mod(const modinfo::ModInfo& mod) {
    fs::path init_path = fs::path(mod.path) / "init.lua";
    if (!fs::exists(init_path)) return false;

    auto& lua = ScriptingServer::lua();

    lua["MOD_NAME"] = mod.name;
    lua["MOD_PATH"] = mod.path;

    ScriptingServer::run_file(init_path.string());

    std::cout << "[ModLoader] successfully loaded '" << mod.name << "'" << std::endl;
    return true;
}

};