#pragma once

#include <functional>

namespace me::cmp::ui {
	struct Interactable {
		std::function<void(entt::registry&)> on_pressed;
		std::function<void(entt::registry&)> on_hovered;
	};
}