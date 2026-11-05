#pragma once

#include <meatengine/ModInfo.hpp>

#include <string>
#include <unordered_map>
#include <vector>

namespace me {

class ModLoader {
private:
    inline static std::string _load_dir;
    inline static std::unordered_map<std::string, ModInfo> _name_map;
    inline static std::vector<std::string> _load_order;

    static bool read_mod_info(const std::string& path, ModInfo& out);
    static std::vector<ModInfo> scan_mods(const std::string& path);
    static std::vector<std::size_t> resolve_order(const std::vector<ModInfo>& mods);
    static bool load_mod(const ModInfo& mod);

public:
    ModLoader() = delete;

    static std::vector<ModInfo> list();

    static const ModInfo* find(const std::string& name);
    static const ModInfo& get(const std::string& name);
    static bool is_loaded(const ModInfo& mi);
    static bool is_loaded(const std::string& name);

    static const std::string& resolve(const ModInfo& mi) noexcept { return mi.name; }

    static bool load(const std::string& dir);
    static bool load();
};

}