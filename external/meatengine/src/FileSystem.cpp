#include <meatengine/FileSystem.hpp>

#include <fstream>
#include <sstream>
#include <filesystem>

namespace me {
    std::string FileSystem::get_user_config_dir(const std::string& app_name) {
            std::string base;
    #ifdef _WIN32
            const char* appdata = std::getenv("APPDATA");
            if (appdata) base = std::string(appdata) + "/" + app_name + "/";
            else base = "./";
    #elif defined(__APPLE__)
            const char* home = std::getenv("HOME");
            if (home) base = std::string(home) + "/Library/Application Support/" + app_name + "/";
            else base = "./";
    #else
            const char* xdg = std::getenv("XDG_CONFIG_HOME");
            const char* home = std::getenv("HOME");
            if (xdg) base = std::string(xdg) + "/" + app_name + "/";
            else if (home) base = std::string(home) + "/.config/" + app_name + "/";
            else base = "./";
    #endif
			try {
				std::filesystem::create_directories(base);
			} catch (...) {

			}
            return base;
        }

    std::string FileSystem::get_full_path(const std::string& path) {
        std::filesystem::path p(path);
        if (p.is_absolute()) {
            return path;
        }

		// else if (path.starts_with("res://")) {
		// 	std::string total_path = path; total_path.erase(0, 6);
		// 	return (std::filesystem::path("" / total_path).string();
		// }
		else if (path.starts_with("user://")) {
			std::string total_path = path; total_path.erase(0, 7);
			return (std::filesystem::path(get_user_config_dir()) / total_path).string();
		}

		return p.string();
    }

    bool FileSystem::get_file_text(const std::string& path, std::string& out) {
        std::ifstream f(get_full_path(path), std::ios::binary);
        if (!f.is_open()) return false;

        std::ostringstream ss;
        ss << f.rdbuf();
        if (f.bad()) return false;

        out = ss.str();
        return true;
    }

    bool FileSystem::save_file(const std::string& path, const std::string& text) {
        const std::string full = get_full_path(path);

        std::filesystem::path p(full);
        if (p.has_parent_path()) {
            std::error_code ec;
            std::filesystem::create_directories(p.parent_path(), ec);
            if (ec) return false;
        }

        std::ofstream f(full, std::ios::binary | std::ios::trunc);
        if (!f.is_open()) return false;

        f.write(text.data(), static_cast<std::streamsize>(text.size()));
        return f.good();
    }

} // namespace me