#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>

#include <meatengine/cmp/Sprite.hpp>
#include <meatengine/cmp/SpriteAnimation.hpp>

namespace me {
    static void update() {
        auto& reg = MainLoop::get_registry();
        auto& window = MainLoop::get_window();
        float dt = MainLoop::get_dt();

        auto view = reg.view<cmp::SpriteAnimation, cmp::Sprite>();

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
}

ME_REGISTER_SYSTEM(
    "engine:sprite_animation:update",
    me::SystemPhase::Update,
    std::numeric_limits<int>::min() + 4,
    me::update
)
