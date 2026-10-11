#pragma once

#include <meatengine/registry_setup.hpp>
#include <meatengine/cmp/ui/FillRect.hpp>
#include <meatengine/cmp/ui/Label.hpp>

//#include <entt/entt.hpp>

namespace me {
	static std::unordered_set<entt::entity>& unlinking() {
		static std::unordered_set<entt::entity> s;
		return s;
	}

	static void mark_dirty(entt::registry& r, entt::entity e) {
		if (auto* fr = r.try_get<cmp::ui::FillRect>(e)) 
			fr->dirty = true;
		if (auto* lbl = r.try_get<cmp::ui::Label>(e))
			lbl->dirty = true;
	}

	static void install(entt::registry& reg) {
		reg.on_construct<cmp::ui::Pressed>().connect<&mark_dirty>();
		reg.on_construct<cmp::ui::Hovered>().connect<&mark_dirty>();
		reg.on_construct<cmp::ui::Disabled>().connect<&mark_dirty>();
		
		reg.on_destroy<cmp::ui::Pressed>().connect<&mark_dirty>();
		reg.on_destroy<cmp::ui::Hovered>().connect<&mark_dirty>();
		reg.on_destroy<cmp::ui::Disabled>().connect<&mark_dirty>();
	}
}

ME_REGISTER_REGISTRY_SETUP(
	"ui",
	[](entt::registry& r) { me::install(r); }
)