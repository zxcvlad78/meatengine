#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>

#include <meatengine/cmp/ZIndex.hpp>
#include <meatengine/cmp/Sprite.hpp>
#include <meatengine/cmp/Transform.hpp>

namespace me {
    static void render() {
        auto& reg = MainLoop::get_registry();
        auto& window = MainLoop::get_window();

        struct Renderable {
            sf::Sprite* sprite;
            sf::Vector2f position;
            sf::Angle rotation;
            sf::Vector2f scale;
            int z_index;
        };

        static std::vector<Renderable> renderables;
        renderables.clear();

        for (auto [entity, transform, sprite] : reg.view<cmp::Transform, cmp::Sprite>().each()) {
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
            if (reg.all_of<cmp::ZIndex>(entity)) {
                z_index = reg.get<cmp::ZIndex>(entity).value;
            }

            // sf::Transform global_transform =  Transform::get_global(reg, entity);
            sf::Vector2f global_pos = transform.position; //global_transform.transformPoint({0.f, 0.f});

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
}

ME_REGISTER_SYSTEM(
    "engine:sprite:render",
    me::SystemPhase::Render,
    std::numeric_limits<int>::min() + 6,
    me::render
)
