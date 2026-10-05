#pragma once
#include "Components.hpp"
#include <entt/entt.hpp>
#include <meatengine/Resources.hpp>
#include <meatengine/ResourceLoader.hpp>
#include <optional>

namespace me {
    struct Sprite {
    private:
        std::optional<sf::Sprite> _sprite;
        entt::resource<me::Texture> _texture{};

    public:
        sf::Vector2f offset{};
        bool center = false;

        Sprite() = default;

        explicit Sprite(entt::resource<me::Texture> tex)
            : _sprite(std::in_place, tex->res)
            , _texture(std::move(tex))
        {}

        void set_texture(entt::resource<me::Texture> tex) {
            if (!tex) {
                _texture = {};
                _sprite.reset();
                return;
            }
            _texture = std::move(tex);
            _sprite.emplace(_texture->res);
        }

        void set_texture(me::Texture& tex) {
            auto h = me::ResourceLoader::find_handle(&tex);
            if (!h) {
                throw std::runtime_error(
                    "Sprite::set_texture: Texture not from ResourceLoader");
            }
            set_texture(entt::resource<me::Texture>{h});
        }

        void clear_texture() {
            _texture = {};
            _sprite.reset();
        }

        bool has_texture() const noexcept {
            return static_cast<bool>(_texture);
        }

        const entt::resource<me::Texture>& get_texture() const noexcept {
            return _texture;
        }

        me::Texture* texture_ptr() const noexcept {
            return _texture ? _texture.operator->() : nullptr;
        }

        bool has_sprite() const noexcept {
            return _sprite.has_value();
        }

        sf::Sprite& ensure_sprite() {
            if (!_sprite) {
                if (!_texture) {
                    throw std::runtime_error("Sprite::ensure_sprite: no texture set");
                }
                _sprite.emplace(_texture->res);
            }
            return *_sprite;
        }

        const sf::Sprite& ensure_sprite() const {
            if (!_sprite) {
                throw std::runtime_error("Sprite::ensure_sprite: sprite not initialized");
            }
            return *_sprite;
        }

        sf::Sprite* sprite_ptr() noexcept { return _sprite ? &*_sprite : nullptr; }
        const sf::Sprite* sprite_ptr() const noexcept { return _sprite ? &*_sprite : nullptr; }
    };

    struct FullScreenScale {
        sf::Vector2f multiplier = {1.f, 1.f};
    };

    struct SpriteAnimation {
        entt::resource<me::SpriteSheet> spritesheet;
        const me::Animation* current_animation = nullptr;
        
        unsigned int current_frame_idx = 0;
        float time_accumulator = 0.0f;
        bool is_playing = true;
        std::string next_anim = ""; 

        bool play(const std::string& animation_name, const std::string& play_next = "") {
            if (spritesheet && spritesheet->animations.contains(animation_name)) {
                is_playing = true;
                current_animation = &spritesheet->animations.at(animation_name);
                current_frame_idx = 0;
                time_accumulator = 0.0f;
                next_anim = play_next;
                return true;
            }
            return false;
        }
    };
}