#pragma once

#include <meatengine/ModInfo.hpp>

namespace me {
    class ModLoader {
    private:
        inline static std::vector<ModInfo> _loaded;

        static bool read_mod_info(const std::string& path, ModInfo& out);
        static std::vector<ModInfo> scan_mods(const std::string& path);
        static std::vector<std::size_t> resolve_order(const std::vector<ModInfo>& mods);
        static bool load_mod(const ModInfo& mod);
    public:
        ModLoader() = delete;

        static std::vector<ModInfo> list();
		static bool init(const std::string& path = "data");
    };
} // namespace me