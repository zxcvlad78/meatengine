#pragma once
#include <string>
#include <sstream>
#include "SFML/Graphics.hpp"
#include "nlohmann/json.hpp"

namespace me::parsing {

    inline int hex_digit(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    }

    inline std::uint8_t clamp_u8(int v) {
        return static_cast<std::uint8_t>(std::clamp(v, 0, 255));
    }

    inline bool hex_to_color(const std::string& s, sf::Color& out) {
        if (s.empty() || s[0] != '#') return false;
        std::string h = s.substr(1);
        for (char c : h) if (hex_digit(c) < 0) return false;

        auto d = [&](std::size_t i) { return hex_digit(h[i]); };

        switch (h.size()) {
            case 3:
                out = sf::Color(
                    clamp_u8(d(0)*17),
                    clamp_u8(d(1)*17),
                    clamp_u8(d(2)*17), 255
                );
                return true;
            case 4:
                out = sf::Color(
                    clamp_u8(d(0)*17),
                    clamp_u8(d(1)*17),
                    clamp_u8(d(2)*17),
                    clamp_u8(d(3)*17)
                );
                return true;
            case 6:
                out = sf::Color(
                    clamp_u8(d(0)*16 + d(1)),
                    clamp_u8(d(2)*16 + d(3)),
                    clamp_u8(d(4)*16 + d(5)),
                    255
                );
                return true;
            case 8:
                out = sf::Color(
                    clamp_u8(d(0)*16 + d(1)),
                    clamp_u8(d(2)*16 + d(3)),
                    clamp_u8(d(4)*16 + d(5)),
                    clamp_u8(d(6)*16 + d(7))
                );
                return true;
        }
        return false;
    }

    inline bool name_to_color(const std::string& s, sf::Color& out) {
        static const std::unordered_map<std::string, sf::Color> table = {
            {"transparent", sf::Color(0,0,0,0)},
            {"black", sf::Color::Black}, {"white", sf::Color::White},
            {"red", sf::Color::Red}, {"green", sf::Color::Green},
            {"blue", sf::Color::Blue}, {"cyan", sf::Color::Cyan},
            {"magenta", sf::Color::Magenta},{"yellow", sf::Color::Yellow},
            {"gray", sf::Color(128,128,128)}, {"grey", sf::Color(128,128,128)},
            {"aqua", sf::Color(0,255,255)},   {"aquamarine", sf::Color(127,255,212)},
            {"blueviolet", sf::Color(138,43,226)}, {"brown", sf::Color(165,42,42)},
            {"burlywood", sf::Color(222,184,135)},
            {"cadetblue", sf::Color(95,158,160)},
            {"chartreuse", sf::Color(127,255,0)},
            {"chocolate", sf::Color(210,105,30)},
        };
        auto it = table.find(s);
        if (it == table.end()) return false;
        out = it->second;
        return true;
    }

    inline sf::Color str_to_color(
        const std::string& str,
        sf::Color fallback = sf::Color::White
    ) {
        if (str.empty()) return fallback;

        sf::Color c;
        if (hex_to_color(str, c))  return c;
        if (name_to_color(str, c)) return c;

        unsigned char first = static_cast<unsigned char>(str[0]);
        if (std::isdigit(first) || str[0] == '-') {
            std::stringstream ss(str);
            int r = 255, g = 255, b = 255, a = 255;
            if (!(ss >> r >> g >> b)) return fallback;
            ss >> a;
            return sf::Color(clamp_u8(r), clamp_u8(g), clamp_u8(b), clamp_u8(a));
        }
        return fallback;
    }

    inline sf::Color vec_to_color(
        const std::vector<int>& v,
        sf::Color fallback = sf::Color::White
    ) {
        if (v.size() < 3 || v.size() > 4) return fallback;
        int a = (v.size() == 4) ? v[3] : 255;
        return sf::Color(clamp_u8(v[0]), clamp_u8(v[1]), clamp_u8(v[2]), clamp_u8(a));
    }

    inline sf::Color str_to_color(const std::vector<std::string>& args) {
        if (args.size() < 3 || args.size() > 4) return sf::Color::Black;
        try {
            return vec_to_color({
                std::stoi(args[0]), std::stoi(args[1]), std::stoi(args[2]),
                args.size() == 4 ? std::stoi(args[3]) : 255
            });
        } catch (const std::exception&) {
            return sf::Color::Black;
        }
    }

    inline std::string color_to_str(const sf::Color& color) {
        return std::to_string(color.r) + " " + std::to_string(color.g) + " " + std::to_string(color.b) + " " + std::to_string(color.a);
    }

    inline std::string color_to_hex(const sf::Color& c) {
        auto hx = [](std::uint8_t v) {
            static const char* d = "0123456789abcdef";
            std::string s(2, '0');
            s[0] = d[(v >> 4) & 0xF];
            s[1] = d[v & 0xF];
            return s;
        };
        return "#" + hx(c.r) + hx(c.g) + hx(c.b) + hx(c.a);
    }
}

namespace sf {
    inline void from_json(const nlohmann::json& j, Color& c) {
        using namespace me::parsing;
        if (j.is_string()) {
            c = str_to_color(j.get<std::string>());
        } else if (j.is_array()) {
            std::vector<int> v;
            v.reserve(j.size());
            for (auto& e : j) {
                if (e.is_number_integer())      v.push_back(e.get<int>());
                else if (e.is_number_unsigned()) v.push_back(static_cast<int>(e.get<unsigned>()));
                else if (e.is_number_float())    v.push_back(static_cast<int>(e.get<double>()));
                else return;
            }
            c = vec_to_color(v, c);
        }
    }

    inline void to_json(nlohmann::json& j, const Color& c) {
        j = me::parsing::color_to_hex(c);
    }
}