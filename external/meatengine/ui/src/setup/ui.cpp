#pragma once

#include <meatengine/ecs/registry_setup.hpp>
#include <meatengine/ui/cmp/FillRect.hpp>
#include <meatengine/ui/cmp/Label.hpp>

#include <entt/entt.hpp>

namespace {
	void mark_dirty(entt::registry& r, entt::entity e) {
		if (auto* fr = r.try_get<me::ui::cmp::FillRect>(e)) 
			fr->dirty = true;
		if (auto* lbl = r.try_get<me::ui::cmp::Label>(e))
			lbl->dirty = true;
	}

	void install(entt::registry& reg) {
		reg.on_construct<me::ui::cmp::Pressed>().connect<&mark_dirty>();
		reg.on_construct<me::ui::cmp::Hovered>().connect<&mark_dirty>();
		reg.on_construct<me::ui::cmp::Disabled>().connect<&mark_dirty>();
		
		reg.on_destroy<me::ui::cmp::Pressed>().connect<&mark_dirty>();
		reg.on_destroy<me::ui::cmp::Hovered>().connect<&mark_dirty>();
		reg.on_destroy<me::ui::cmp::Disabled>().connect<&mark_dirty>();
	}
}

ME_REGISTER_REGISTRY_SETUP(
	"engine:ui",
	[](entt::registry& r) { install(r); }
)