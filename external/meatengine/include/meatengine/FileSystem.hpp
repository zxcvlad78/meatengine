#pragma once
#include <string>

namespace me {
	class FileSystem {
    private:

    public:
        FileSystem() = delete;

		static std::string get_user_config_dir(const std::string& app_name = "MeatEngine");
        static std::string get_full_path(const std::string& path);
		
		static bool get_file_text(const std::string& path, std::string& out);
		static bool save_file(const std::string& path, const std::string& text);
    };
}