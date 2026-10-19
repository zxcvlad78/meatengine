#pragma once

#include <optional>
#include <SFML/Graphics/Text.hpp>

#include <entt/resource/resource.hpp>
#include <entt/entity/registry.hpp>
#include <entt/entity/entity.hpp>

#include <meatengine/Resources.hpp>
#include <meatengine/ResourceLoader.hpp>
#include <meatengine/cmp/ui/States.hpp>

namespace me::cmp::ui {

    struct Label {
    private:
        entt::resource<me::Font> _font{};

        void sync_origin() {
            if (!text) return;
            text->setOrigin(text->getLocalBounds().position);
        }

    public:
        std::optional<sf::Text> text;
        bool dirty = true;

        entt::resource<me::Font> get_font() const { return _font; }

        void set_font(entt::resource<me::Font> font) {
            if (!font) {
                _font = {};
                text.reset();
                dirty = true;
                return;
            }
            _font = std::move(font);
            text.emplace(_font->res);
            sync_origin();
            dirty = true;
        }

        void set_font(me::Font& font) {
            auto h = me::ResourceLoader::find_handle(&font);
            if (!h) {
                throw std::runtime_error(
                    "Label::set_font: Font not from ResourceLoader"
                );
            }
            set_font(entt::resource<me::Font>{h});
        }

        bool has_text() const noexcept { return text.has_value(); }

        sf::Text* text_ptr() noexcept { return text ? &*text : nullptr; }
        const sf::Text* text_ptr() const noexcept { return text ? &*text : nullptr; }

        void set_string(const std::string& s) {
            if (!text) return;
            text->setString(sf::String::fromUtf8(s.begin(), s.end()));
            sync_origin();
            dirty = true;
        }

        void set_character_size(unsigned int size) {
            if (!text) return;
            text->setCharacterSize(size);
            sync_origin();
            dirty = true;
        }

        void set_color(sf::Color c) {
            if (!text) return;
            text->setFillColor(c);
            dirty = true;
        }

        void set_position(sf::Vector2f p) {
            if (!text) return;
            text->setPosition(p);
        }

        Label() = default;

        explicit Label(entt::resource<me::Font> font) {
            set_font(std::move(font));
        }

        explicit Label(me::Font& font) {
            set_font(font);
        }
    };

}