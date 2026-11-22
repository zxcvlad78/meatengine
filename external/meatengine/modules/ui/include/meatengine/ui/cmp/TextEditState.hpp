#pragma once

#include <string>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <entt/resource/resource.hpp>

#include <meatengine/assets/Resources.hpp>
#include <meatengine/assets/ResourceLoader.hpp>

namespace me::ui::cmp {

    struct TextEditState {
        std::u32string value;
        std::u32string placeholder;
        std::size_t cursor = 0;
        std::size_t anchor = 0; // selection anchor; if != cursor => selection
        std::size_t preferred_column = 0;
        bool multiline = false;
        bool read_only = false;
        bool password = false;
        char32_t password_char = U'*';

        entt::resource<me::Font> font{};
        std::optional<sf::Text> text;
        sf::RectangleShape cursor_shape;
        sf::RectangleShape selection_shape;
        unsigned int character_size = 18;
        sf::Color text_color = sf::Color::Black;
        sf::Color placeholder_color = sf::Color(140, 140, 140);
        sf::Color cursor_color = sf::Color::Black;
        sf::Color selection_color = sf::Color(80, 120, 200, 120);

        float cursor_blink = 0.f;
        bool cursor_visible = true;
        bool dirty = true;

        float scroll_x = 0.f;
        float scroll_y = 0.f;

        TextEditState() {
            cursor_shape.setSize({1.f, 18.f});
            cursor_shape.setFillColor(cursor_color);
            selection_shape.setFillColor(selection_color);
            auto f = ResourceLoader::get_default<me::Font>();
            if (f) set_font(entt::resource<me::Font>{f});
        }

        void set_font(entt::resource<me::Font> f) {
            if (!f) return;
            font = std::move(f);
            text.emplace(font->res);
            text->setCharacterSize(character_size);
            text->setFillColor(text_color);
            dirty = true;
        }

        bool has_selection() const { return cursor != anchor; }
        std::size_t sel_min() const { return std::min(cursor, anchor); }
        std::size_t sel_max() const { return std::max(cursor, anchor); }

        void clear_selection() { anchor = cursor; }

        void clamp() {
            if (cursor > value.size()) cursor = value.size();
            if (anchor > value.size()) anchor = value.size();
        }

        void delete_selection() {
            if (!has_selection()) return;
            auto a = sel_min(), b = sel_max();
            value.erase(a, b - a);
            cursor = a;
            anchor = a;
            dirty = true;
        }

        void insert_text(const std::u32string& s) {
            if (read_only || s.empty()) return;
            delete_selection();
            value.insert(cursor, s);
            cursor += s.size();
            anchor = cursor;
            dirty = true;
        }

        void backspace() {
            if (read_only) return;
            if (has_selection()) { delete_selection(); return; }
            if (cursor == 0) return;
            value.erase(cursor - 1, 1);
            cursor--;
            anchor = cursor;
            dirty = true;
        }

        void del() {
            if (read_only) return;
            if (has_selection()) { delete_selection(); return; }
            if (cursor >= value.size()) return;
            value.erase(cursor, 1);
            dirty = true;
        }

        void move_left(bool select, bool word) {
            if (cursor == 0) { if (!select) anchor = cursor; return; }
            if (word) {
                while (cursor > 0 && value[cursor - 1] == U' ') cursor--;
                while (cursor > 0 && value[cursor - 1] != U' ' && value[cursor - 1] != U'\n') cursor--;
            } else {
                cursor--;
            }
            if (!select) anchor = cursor;
        }
        void move_right(bool select, bool word) {
            if (cursor >= value.size()) { if (!select) anchor = cursor; return; }
            if (word) {
                while (cursor < value.size() && value[cursor] != U' ' && value[cursor] != U'\n') cursor++;
                while (cursor < value.size() && value[cursor] == U' ') cursor++;
            } else {
                cursor++;
            }
            if (!select) anchor = cursor;
        }

        void select_all() {
            anchor = 0;
            cursor = value.size();
        }

        std::u32string selected_text() const {
            if (!has_selection()) return {};
            return value.substr(sel_min(), sel_max() - sel_min());
        }

        void set_value_utf8(const std::string& s) {
            value = sf::String::fromUtf8(s.begin(), s.end()).toUtf32();
            cursor = std::min(cursor, value.size());
            anchor = std::min(anchor, value.size());
            dirty = true;
        }
        std::string get_value_utf8() const {
            auto u8 = sf::String(value).toUtf8();
            return std::string(u8.begin(), u8.end());
        }
    };

    struct LineEdit { };
    struct TextEdit { };

}

