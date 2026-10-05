#include <meatengine/Generic.hpp>
#include <meatengine/sprite/Systems.hpp>
#include <meatengine/render/Components.hpp>

namespace me::SpriteSystems {

    void update(entt::registry& registry, sf::RenderWindow& window, float dt) {
        handle_fullscreen_scale(registry, window);
        update_animation(registry, dt);
    }

    void handle_fullscreen_scale(entt::registry& registry, sf::RenderWindow& window) {
        auto view = registry.view<Sprite, Transform, FullScreenScale>();
        sf::Vector2f window_sizef = static_cast<sf::Vector2f>(window.getSize());

        for (auto [e, s, t, fsc] : view.each()) {
            if (!s.has_sprite()) continue;

            auto texture_size = s.sprite_ptr()->getTextureRect().size;
            if (texture_size.x == 0 || texture_size.y == 0) continue;

            sf::Vector2f target_scale = {
                window_sizef.x / static_cast<float>(texture_size.x) * fsc.multiplier.x,
                window_sizef.y / static_cast<float>(texture_size.y) * fsc.multiplier.y
            };

            t.scale = target_scale;
        }
    }

    void render(entt::registry& registry, sf::RenderWindow& window) {
        struct Renderable {
            sf::Sprite* sprite;
            sf::Vector2f position;
            sf::Angle rotation;
            sf::Vector2f scale;
            int z_index;
        };

        static std::vector<Renderable> renderables;
        renderables.clear();

        for (auto [entity, transform, sprite] : registry.view<Transform, Sprite>().each()) {
            sf::Sprite* sf_sprite = sprite.sprite_ptr();
            if (!sf_sprite) continue;

            if (sprite.center) {
                auto tex_rect = sf_sprite->getTextureRect();
                sprite.offset = {
                    -static_cast<float>(tex_rect.size.x) / 2.f,
                    -static_cast<float>(tex_rect.size.y) / 2.f
                };
            }

            int z_index = 0;
            if (registry.all_of<ZIndex>(entity)) {
                z_index = registry.get<ZIndex>(entity).value;
            }

            sf::Transform global_transform = Transform::get_global(registry, entity);
            sf::Vector2f global_pos = global_transform.transformPoint({0.f, 0.f});

            sf::Angle global_rotation = transform.rotation;
            sf::Vector2f global_scale = transform.scale;

            Renderable renderable;
            renderable.sprite = sf_sprite;
            renderable.position = global_pos + sprite.offset;
            renderable.rotation = global_rotation;
            renderable.scale = global_scale;
            renderable.z_index = z_index;

            renderables.push_back(renderable);
        }

        std::sort(renderables.begin(), renderables.end(),
            [](const Renderable& a, const Renderable& b) {
                return a.z_index < b.z_index;
            });

        for (const auto& renderable : renderables) {
            renderable.sprite->setPosition(renderable.position);
            renderable.sprite->setRotation(renderable.rotation);
            renderable.sprite->setScale(renderable.scale);

            window.draw(*renderable.sprite);
        }
    }

    void update_animation(entt::registry& registry, float dt) {
        auto view = registry.view<SpriteAnimation, Sprite>();

        for (auto [entity, sprite_anim, sprite] : view.each()) {
            if (!sprite_anim.is_playing || !sprite_anim.current_animation
                || sprite_anim.current_animation->frames.empty()) {
                continue;
            }

            sf::Sprite* sf_sprite = sprite.sprite_ptr();
            if (!sf_sprite) continue;

            sprite_anim.time_accumulator += dt;
            float frame_duration = 1.0f / sprite_anim.current_animation->fps;

            while (sprite_anim.time_accumulator >= frame_duration) {
                sprite_anim.time_accumulator -= frame_duration;
                size_t next_frame = sprite_anim.current_frame_idx + 1;

                if (next_frame >= sprite_anim.current_animation->frames.size()) {
                    if (sprite_anim.current_animation->is_looping) {
                        sprite_anim.current_frame_idx = 0;
                    } else {
                        if (sprite_anim.next_anim.empty()) {
                            sprite_anim.is_playing = false;
                        } else {
                            sprite_anim.play(sprite_anim.next_anim);
                        }
                        break;
                    }
                } else {
                    sprite_anim.current_frame_idx = next_frame;
                }
            }

            const me::Animation::FrameData& frame =
                sprite_anim.current_animation->frames[sprite_anim.current_frame_idx];

            sf_sprite->setTextureRect(
                sf::IntRect({frame.x, frame.y}, {frame.w, frame.h})
            );
        }
    }

} // namespace me::SpriteSystems