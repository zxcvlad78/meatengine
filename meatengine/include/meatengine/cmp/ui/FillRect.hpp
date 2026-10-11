#pragma once
#include <meatengine/cmp/ui/States.hpp>
#include <meatengine/Resources.hpp>
#include <entt/entity/registry.hpp>
#include <entt/entity/entity.hpp>

namespace me::cmp::ui {
    struct FillRect {
		entt::resource<me::StyleBox> stylebox;
		sf::RectangleShape shape;
		bool foreground = true;

		bool dirty = true;

		void update_fill(entt::registry& reg, entt::entity e) {
			if (!dirty) return;
			if (!stylebox) return;


			shape.setOutlineColor(stylebox->get_value<sf::Color>("outline_color", sf::Color::White));
			shape.setOutlineThickness(stylebox->get_value<float>("outline_thickness", 0.0f));

			if (reg.all_of<Disabled>(e))
				shape.setFillColor(stylebox->get_value<sf::Color>("disabled_color", sf::Color::White));
			else if (reg.all_of<Pressed>(e))
				shape.setFillColor(stylebox->get_value<sf::Color>("pressed_color", sf::Color::White));
			else if (reg.all_of<Hovered>(e))
				shape.setFillColor(stylebox->get_value<sf::Color>("hovered_color", sf::Color::White));
			else {
				if (foreground) shape.setFillColor(stylebox->get_value<sf::Color>("foreground_color", sf::Color::White));
				else shape.setFillColor(stylebox->get_value<sf::Color>("background_color", sf::Color::White));
			}
		
			dirty = false;
		}
	};
}