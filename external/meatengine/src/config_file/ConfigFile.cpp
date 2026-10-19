#include <meatengine/config_file/ConfigFile.hpp>
#include <meatengine/FileSystem.hpp>
#include <filesystem>

namespace me {
    void ConfigFile::set_save_path(const std::string& new_path) { _save_path = new_path; }
    std::string ConfigFile::get_save_path() const { return _save_path; }

    bool ConfigFile::parse_line(const std::string& line, std::string& key, std::string& value) {
        size_t eq = line.find('=');
        if (eq == std::string::npos || eq == 0 || eq == line.size() - 1)
            return false;
        key = line.substr(0, eq);
        value = line.substr(eq + 1);
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        return true;
    }

    bool ConfigFile::load(const std::string& path) {
        std::string full_path;
        if (path.empty()) full_path = me::FileSystem::get_full_path(_save_path);
        else full_path = me::FileSystem::get_full_path(path);

        if (!std::filesystem::exists(full_path)) {
            return false;
        }
        std::ifstream file(full_path);
        if (!file.is_open()) {
            return false;
        }

        values.clear();
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#')
                continue;
            std::string key, value;
            if (parse_line(line, key, value)) {
                values[key] = value;
            }
        }
        return true;
    }

    bool ConfigFile::save(const std::string& path) const {
        std::string full_path;
        if (path.empty()) full_path = me::FileSystem::get_full_path(_save_path);
        else full_path = me::FileSystem::get_full_path(path);

        std::ofstream file(full_path);
        if (!file.is_open())
            return false;

        for (const auto& [key, value] : values) {
            file << key << "=" << value << "\n";
        }
        return true;
    }


    // bool
    void ConfigFile::set(const std::string& key, bool value) {
        values[key] = value ? "1" : "0";
    }
    bool ConfigFile::get(const std::string& key, bool default_value) const {
        auto it = values.find(key);
        if (it == values.end())
            return default_value;
        std::string v = it->second;
        if (v == "true" || v == "1" || v == "yes" || v == "on")
            return true;
        if (v == "false" || v == "0" || v == "no" || v == "off")
            return false;
        return default_value;
    }


    // string
    void ConfigFile::set(const std::string& key, const std::string value) {
        values[key] = value;
    }
    std::string ConfigFile::get(const std::string& key, const std::string default_value) const {
        auto it = values.find(key);
        return (it != values.end()) ? it->second : default_value;
    }

    // int

    void ConfigFile::set(const std::string& key, int value) {
        values[key] = std::to_string(value);
    }
    int ConfigFile::get(const std::string& key, int default_value) const {
        auto it = values.find(key);
        if (it == values.end())
            return default_value;
        
        
        return std::stoi(it->second);
    }

    // uint

    void ConfigFile::set(const std::string& key, unsigned int value) {
        values[key] = std::to_string(value);
    }
    unsigned int ConfigFile::get(const std::string& key, unsigned int default_value) const {
        auto it = values.find(key);
        if (it == values.end())
            return default_value;
        
        
        return std::stoi(it->second);
    }

    // float
    void ConfigFile::set(const std::string& key, float value) {
        values[key] = std::to_string(value);
    }
    float ConfigFile::get(const std::string& key, float default_value) const {
        auto it = values.find(key);
        if (it == values.end())
            return default_value;
        
        
        return std::stof(it->second);
    }

    // char[]

    void ConfigFile::set(const std::string& key, const char* value) {
        values[key] = value;
    }

    std::string ConfigFile::get(const std::string& key, const char* default_value) const {
        auto it = values.find(key);
        if (it != values.end()) return it->second;
        return default_value ? default_value : "";
    }
} // namespace me