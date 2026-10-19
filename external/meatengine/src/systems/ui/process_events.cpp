#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>
#include <meatengine/ctx/InputState.hpp>

#include <meatengine/cmp/Parent.hpp>
#include <meatengine/cmp/GlobalBounds.hpp>

#include <meatengine/cmp/ui/ComputedNode.hpp>
#include <meatengine/cmp/ui/Interaction.hpp>
#include <meatengine/cmp/ui/FocusPolicy.hpp>
#include <meatengine/cmp/ui/OnEvent.hpp>
#include <meatengine/cmp/ui/States.hpp>

#include <algorithm>
#include <vector>

namespace {

using namespace me;


void sync_state_tags(
    entt::registry& r,
    entt::entity e,
    me::cmp::ui::Interaction state
) {
    using me::cmp::ui::Interaction;

    const bool hovered = (state == Interaction::Hovered || state == Interaction::Pressed);
    const bool pressed = (state == Interaction::Pressed);

    if (hovered) r.emplace_or_replace<me::cmp::ui::Hovered>(e);
    else r.remove<me::cmp::ui::Hovered>(e);

    if (pressed) r.emplace_or_replace<me::cmp::ui::Pressed>(e);
    else r.remove<me::cmp::ui::Pressed>(e);
}

void bubble_event(entt::registry& r, entt::entity e, me::cmp::ui::Event& ev) {
    while (e != entt::null && r.valid(e)) {
        if (auto* h = r.try_get<me::cmp::ui::OnEvent>(e)) {
            h->cb(r, e, ev);
            if (ev.handled) return;
        }
        auto* p = r.try_get<me::cmp::Parent>(e);
        e = (p && p->entity != entt::null && r.valid(p->entity)) ? p->entity : entt::null;
    }
}

entt::entity pick_topmost(entt::registry& r, sf::Vector2f world_pos) {
    entt::entity best = entt::null;
    uint32_t best_stack = 0;

    for (auto [e, gb] : r.view<me::cmp::GlobalBounds>().each()) {
        if (!gb.rect.contains(world_pos)) continue;

        auto* cn = r.try_get<me::cmp::ui::ComputedNode>(e);
        uint32_t stack = cn ? cn->stack_index : 0u;

        const bool interactive = r.any_of<me::cmp::ui::Interaction>(e);
        if (!interactive) continue;

        if (best == entt::null || stack >= best_stack) {
            best = e;
            best_stack = stack;
        }
    }
    return best;
}

void run() {
    auto& r = me::MainLoop::get_registry();
    auto& in = r.ctx().get<me::ctx::InputState>();

    using me::cmp::ui::Interaction;
    using me::cmp::ui::Event;
    using me::cmp::ui::EventType;

    entt::entity target = pick_topmost(r, in.mouse_pos);

    while (target != entt::null) {
        auto* fp = r.try_get<me::cmp::ui::FocusPolicy>(target);
        if (fp && fp->kind == me::cmp::ui::FocusPolicy::Kind::None) {
            auto* p = r.try_get<me::cmp::Parent>(target);
            target = (p && r.valid(p->entity)) ? p->entity : entt::null;
            continue;
        }
        break;
    }

    static entt::entity prev_hover = entt::null;

    if (target != prev_hover) {
        if (prev_hover != entt::null && r.valid(prev_hover)) {
            Event ev{EventType::Unhover, in.mouse_pos, prev_hover, false};
            bubble_event(r, prev_hover, ev);

            if (auto* i = r.try_get<Interaction>(prev_hover))
                *i = Interaction::None;
            sync_state_tags(r, prev_hover, Interaction::None);
        }
        if (target != entt::null) {
            Event ev{EventType::Hover, in.mouse_pos, target, false};
            bubble_event(r, target, ev);
        }
        prev_hover = target;
    }

    if (target != entt::null) {
        if (auto* i = r.try_get<Interaction>(target)) {
            if (in.mouse_down)
                *i = Interaction::Pressed;
            else
                *i = Interaction::Hovered;
        }
        sync_state_tags(r, target,
            in.mouse_down ? Interaction::Pressed : Interaction::Hovered);
    }

    static entt::entity press_target = entt::null;

    if (in.mouse_just_pressed && target != entt::null) {
        press_target = target;
        Event ev{EventType::Press, in.mouse_pos, target, false};
        bubble_event(r, target, ev);
    }

    if (in.mouse_just_released) {
        if (press_target != entt::null && press_target == target) {
            Event ev{EventType::Click, in.mouse_pos, target, false};
            bubble_event(r, target, ev);
        }
        if (press_target != entt::null && r.valid(press_target)) {
            Event ev{EventType::Release, in.mouse_pos, press_target, false};
            bubble_event(r, press_target, ev);
        }
        press_target = entt::null;
    }
}

}

ME_REGISTER_SYSTEM("engine:ui:process_events",
    me::SystemPhase::Input,
    10,
    run
)