#include <meatengine/ecs/SystemRegistry.hpp>
#include <meatengine/app/MainLoop.hpp>
#include <meatengine/net/Network.hpp>

#include <meatengine/debug/Console.hpp>

#include <godlike/cmp/MoveSpeed.hpp>
#include <godlike/cmp/PlayerInput.hpp>
#include <meatengine/physics/cmp/Velocity.hpp>
#include <meatengine/net/cmp/Authority.hpp>
#include <meatengine/ecs/PackedEntity.hpp>

static void player_input() {
	auto& reg = me::MainLoop::get_registry();
	auto& window = me::MainLoop::get_window();

	if (!window.hasFocus()) return;

	auto view = reg.view<godlike::cmp::MoveSpeed, me::physics::cmp::Velocity, godlike::cmp::PlayerInput>();

	for (auto [entity, movespeed, velocity] : view.each()) {
		if (!me::net::Network::is_authority(entity)) continue;

		velocity.linear.x = 0.0f;
		velocity.linear.y = 0.0f;

		if (me::debug::Console::get_instance().is_visible()) { continue; }
		
		velocity.linear.y -= sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
		velocity.linear.y += sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
		velocity.linear.x -= sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
		velocity.linear.x += sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
		
		velocity.linear.x *= movespeed.value;
		velocity.linear.y *= movespeed.value;

		if (velocity.linear.x == 0.0f && velocity.linear.y == 0.0f) continue;
		if (velocity.linear.x != 0.0f && velocity.linear.y != 0.0f) {
			velocity.linear.x *= 0.70710678118f;
			velocity.linear.y *= 0.70710678118f;
		}


		sf::Vector2f direction;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
		{
			direction.x = 1.0f;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
		{
			direction.x = -1.0f;

		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
		{
			direction.y = 1.0f;

		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
		{
			direction.y = -1.0f;

		}

		if (direction.x != 0.0f && direction.y != 0.0f)
		{
			auto bullet_entity = me::PackedEntity::spawn("godlike:bullet", reg);
			auto* bullet_velocity = reg.try_get<me::physics::cmp::Velocity>(bullet_entity);
			if (bullet_velocity != nullptr)
			{
				bullet_velocity->linear.x = direction.x * 100.0f;
				bullet_velocity->linear.y = direction.y * 100.0f;
			}

		}
		
	}
}


ME_REGISTER_SYSTEM(
    "godlike:player_input",
    me::SystemPhase::Input,
    0,
    player_input
)
