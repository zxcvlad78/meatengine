#pragma once

#include <meatengine/cmp/Transform.hpp>
#include <meatengine/cmp/Velocity.hpp>
#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>

namespace me {
	static void movement() {
		auto& reg = MainLoop::get_registry();
		float dt = MainLoop::get_fixed_dt();

		for (auto [e, t, v] : reg.view<cmp::Transform, cmp::Velocity>().each()) {
			t.position.x += v.linear.x * dt;
			t.position.y += v.linear.y * dt;

			//t.rotation += v.angular * dt;
		}
	}
}

ME_REGISTER_SYSTEM(
    "engine:movement",
    me::SystemPhase::FixedUpdate,
    std::numeric_limits<int>::min(),
    me::movement
)
