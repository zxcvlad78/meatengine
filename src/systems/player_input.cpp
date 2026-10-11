#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>

#include <meatengine/console/Console.hpp>

#include <godlike/components/Common.hpp>
#include <meatengine/cmp/Velocity.hpp>

namespace godlike {
	static void player_input() {
		auto& reg = me::MainLoop::get_registry();

		auto view = reg.view<components::MoveSpeed, me::cmp::Velocity, components::PlayerInput>();
		
		for (auto [entity, movespeed, velocity] : view.each()) {
			velocity.linear.x = 0.0f;
			velocity.linear.y = 0.0f;
	
			if (me::Console::get_instance().is_visible()) { continue; }
			
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
			
		}
	}
}

ME_REGISTER_SYSTEM(
    "godlike:player_input",
    me::SystemPhase::Input,
    0,
    godlike::player_input
)
