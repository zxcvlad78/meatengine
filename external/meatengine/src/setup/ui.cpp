#pragma once

#include <meatengine/registry_setup.hpp>
#include <meatengine/cmp/ui/FillRect.hpp>
#include <meatengine/cmp/ui/Label.hpp>

//#include <entt/entt.hpp>

namespace {
	void mark_dirty(entt::registry& r, entt::entity e) {
		if (auto* fr = r.try_get<me::cmp::ui::FillRect>(e)) 
			fr->dirty = true;
		if (auto* lbl = r.try_get<me::cmp::ui::Label>(e))
			lbl->dirty = true;
	}

	void install(entt::registry& reg) {
		reg.on_construct<me::cmp::ui::Pressed>().connect<&mark_dirty>();
		reg.on_construct<me::cmp::ui::Hovered>().connect<&mark_dirty>();
		reg.on_construct<me::cmp::ui::Disabled>().connect<&mark_dirty>();
		
		reg.on_destroy<me::cmp::ui::Pressed>().connect<&mark_dirty>();
		reg.on_destroy<me::cmp::ui::Hovered>().connect<&mark_dirty>();
		reg.on_destroy<me::cmp::ui::Disabled>().connect<&mark_dirty>();
	}
}

ME_REGISTER_REGISTRY_SETUP(
	"engine:ui",
	[](entt::registry& r) { install(r); }
)