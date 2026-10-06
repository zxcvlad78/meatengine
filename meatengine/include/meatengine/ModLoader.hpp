#pragma once

#include <meatengine/modinfo.hpp>

namespace me {
    class ModLoader {
    private:
        static bool read_mod_info(const std::string& path, modinfo::ModInfo& out);
        static std::vector<modinfo::ModInfo> scan_mods(const std::string& path);
        static std::vector<std::size_t> resolve_order(const std::vector<modinfo::ModInfo>& mods);
        static bool load_mod(const modinfo::ModInfo& mod);
    public:
        ModLoader() = delete;

		static bool init_dir(const std::string& path = "data");
    };
} // namespace me