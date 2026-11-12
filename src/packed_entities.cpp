#include <meatengine/ecs/PackedEntity.hpp>
#include <godlike/cmp/DamageDealer.hpp>
#include <godlike/cmp/Health.hpp>
#include <meatengine/physics/cmp/RectCollider.hpp>
#include <meatengine/physics/cmp/Velocity.hpp>
#include <meatengine/spatial/cmp/Transform.hpp>
#include <meatengine/graphics/cmp/Sprite.hpp>
#include <meatengine/assets/ResourceLoader.hpp>
#include <entt/entt.hpp>

entt::entity spawn_bullet(entt::registry &reg)
{
    using namespace godlike::cmp;
    using namespace me::physics::cmp;
    using namespace me::spatial::cmp;
    using namespace me::graphics::cmp;
    using namespace me;

    auto projectile = reg.create();
    auto& rect_collider = reg.emplace<RectCollider>(projectile);
    rect_collider.size = {24, 24};
    auto& damage_dealer = reg.emplace<DamageDealer>(projectile);
    reg.emplace<Transform>(projectile);
    reg.emplace<Velocity>(projectile);
    auto& sprite = reg.emplace<Sprite>(projectile, ResourceLoader::load<Texture>("data/core/res/textures/bullet.png"));

    rect_collider.on_collision = [&reg, projectile](entt::entity e)
    {
        if (auto* damage_dealer = reg.try_get<DamageDealer>(projectile))
        {
            if (auto* health = reg.try_get<godlike::cmp::Health>(e))
            {
                health->points -= damage_dealer->amount;
            }
        }

    };

    return projectile;
}

namespace godlike::packed_entities
{
    void init()
    {
        me::PackedEntity::register_pe("godlike:bullet", spawn_bullet);
    }
}