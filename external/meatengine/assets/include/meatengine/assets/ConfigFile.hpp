#pragma once

#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <iostream>

namespace me {
    class ConfigFile {
    private:
        std::unordered_map<std::string, std::string> values;
        std::string _save_path;
        bool parse_line(const std::string& line, std::string& key, std::string& value);
    public:
        ConfigFile() = default;
        ConfigFile(std::string save_path) : _save_path(save_path) { }

        void set_save_path(const std::string& new_path);
        std::string get_save_path() const;

        bool load(const std::string& path = "");
        bool save(const std::string& path = "") const;

        void set(const std::string& key, bool value);
        bool get(const std::string& key, bool default_value = false) const;

        void set(const std::string& key, const std::string value);
        std::string get(const std::string& key, const std::string default_value = "") const;

        void set(const std::string& key, int value);
        int get(const std::string& key, int default_value = 0) const;

        void set(const std::string& key, unsigned int value);
        unsigned int get(const std::string& key, unsigned int default_value = 0) const;

        void set(const std::string& key, float value);
        float get(const std::string& key, float default_value = 0.f) const;

        void set(const std::string& key, const char* value);
        std::string get(const std::string& key, const char* default_value = "") const;
    };
} // namespace me