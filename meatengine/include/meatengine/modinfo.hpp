#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <charconv>
#include <stdexcept>

namespace me::modinfo {
    struct Version {
        uint32_t major = 0; uint32_t minor = 0; uint32_t patch = 0;

        static Version parse(const std::string& s) {
            Version v;
            uint32_t parts[3] = {0, 0, 0};
            int idx = 0;

            std::size_t start = 0;
            while (start <= s.size() && idx < 3) {
                std::size_t dot = s.find('.', start);
                std::string tok = s.substr(start, dot == std::string::npos ? std::string::npos : dot - start);

                uint32_t val = 0;
                if (!tok.empty()) {
                    auto [p, ec] = std::from_chars(tok.data(), tok.data() + tok.size(), val);
                    if (ec != std::errc{} || p != tok.data() + tok.size())
                        throw std::runtime_error("[ModLoader] bad version component '" + tok + "' in '" + s + "'");
                }
                parts[idx++] = val;
                if (dot == std::string::npos) break;
                start = dot + 1;
            }

            v.major = parts[0];
            v.minor = parts[1];
            v.patch = parts[2];
            return v;
        }

        std::string to_string() const {
            return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
        }

        bool operator<(const Version& o) const noexcept {
            if (major != o.major) return major < o.major;
            if (minor != o.minor) return minor < o.minor;
            return patch < o.patch;
        }

        bool operator==(const Version& o) const noexcept {
            return major == o.major && minor == o.minor && patch == o.patch;
        }

        bool operator>=(const Version& o) const noexcept { return !(*this < o); }
        bool operator<=(const Version& o) const noexcept { return *this < o || *this == o; }
    };

    enum class DependencyType { Required, Optional, Conflict, LoadBefore };

    struct Dependency {
        DependencyType type = DependencyType::Required;
        std::string name;
        bool has_version = false;
        std::string op;
        Version version;

        static Dependency parse(const std::string& s) {
            Dependency d;

            std::size_t pos = s.find_first_not_of(" \t");
            if (pos == std::string::npos)
                throw std::runtime_error("[ModLoader] empty dependency string");

            char c = s[pos];
            if (c == '?') { d.type = DependencyType::Optional; ++pos; }
            else if (c == '!') { d.type = DependencyType::Conflict; ++pos; }
            else if (c == '~') { d.type = DependencyType::LoadBefore; ++pos; }

            pos = s.find_first_not_of(" \t", pos);
            if (pos == std::string::npos)
                throw std::runtime_error("[ModLoader] malformed dependency '" + s + "'");

            std::size_t nameEnd = s.find_first_of(" \t", pos);
            if (nameEnd == std::string::npos) {
                d.name = s.substr(pos);
                return d;
            }
            d.name = s.substr(pos, nameEnd - pos);

            std::size_t rest = s.find_first_not_of(" \t", nameEnd);
            if (rest == std::string::npos) return d;

            std::string tail = s.substr(rest);
            std::size_t sp = tail.find_first_of(" \t");
            if (sp == std::string::npos) {
                d.op = "==";
                d.version = Version::parse(tail);
                d.has_version = true;
                return d;
            }

            d.op = tail.substr(0, sp);
            std::size_t vs = tail.find_first_not_of(" \t", sp);
            if (vs == std::string::npos)
                throw std::runtime_error("[ModLoader] missing version in '" + s + "'");

            d.version = Version::parse(tail.substr(vs));
            d.has_version = true;
            return d;
        }
    };

    struct ModInfo {
        std::string name;
        Version version;
        std::string path; // abs path
        std::vector<Dependency> dependencies;
    };
};