#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>

#include <meatengine/cmp/ui/States.hpp>
#include <meatengine/cmp/ui/FillRect.hpp>
#include <meatengine/cmp/ui/Interactable.hpp>
#include <meatengine/ctx/InputState.hpp>

namespace me {
    static void process_events() {
        auto& reg = MainLoop::get_registry();
        auto& window = MainLoop::get_window();

		auto& in = reg.ctx().get<ctx::InputState>();

        for (auto [e, fr, itr] : reg.view<cmp::ui::FillRect, cmp::ui::Interactable>().each()) {
            if (reg.all_of<cmp::ui::Disabled>(e)) {
                if (reg.all_of<cmp::ui::Hovered>(e)) reg.remove<cmp::ui::Hovered>(e);
                if (reg.all_of<cmp::ui::Pressed>(e)) reg.remove<cmp::ui::Pressed>(e);
                continue;
            }

            bool hit = fr.shape.getGlobalBounds().contains(in.mouse_pos);
            bool was = reg.all_of<cmp::ui::Hovered>(e);
            if (hit && !was) {
                reg.emplace_or_replace<cmp::ui::Hovered>(e);
                if (itr.on_hovered) itr.on_hovered(reg);
            } else if (!hit && was) {
                reg.remove<cmp::ui::Hovered>(e);
            }

            bool hovered = reg.all_of<cmp::ui::Hovered>(e);
            bool pressed = reg.all_of<cmp::ui::Pressed>(e);

            if (hovered && in.mouse_just_pressed) {
                reg.emplace_or_replace<cmp::ui::Pressed>(e);
                continue;
            }
			
            if (pressed && !in.mouse_down) {
                reg.remove<cmp::ui::Pressed>(e);
                if (hovered && itr.on_pressed) itr.on_pressed(reg);
            }

        }

        in.mouse_just_pressed = false;
        in.mouse_just_released = false;
    }
}

ME_REGISTER_SYSTEM(
    "engine:ui:process_events",
    me::SystemPhase::Input,
    std::numeric_limits<int>::min() + 7,
    me::process_events
)