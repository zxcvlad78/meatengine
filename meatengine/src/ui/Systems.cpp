#include <meatengine/Generic.hpp>
#include <meatengine/render/Components.hpp>
#include <meatengine/ui/Components.hpp>
#include <meatengine/ui/Systems.hpp>


namespace me::ui::Systems {
	void update(entt::registry& reg) {
		for (auto [e, fr] : reg.view<FillRect>().each()) {
            fr.update_fill(reg, e);
            
		}
		for (auto [e, label] : reg.view<Label>().each()) {
            label.update_fill(reg, e);
            
		}

	};

    void render(entt::registry& reg, sf::RenderWindow& window) {
        std::vector<std::pair<int, entt::entity>> order;
        for (auto [e, fr] : reg.view<FillRect>().each()) {
            int z = 0;
            if (auto* zi = reg.try_get<ZIndex>(e)) z = zi->value;
            order.emplace_back(z, e);
        }
        std::stable_sort(order.begin(), order.end(), [](auto& a, auto& b) {
			return a.first < b.first;
		});

        for (auto [z, e] : order) {
            auto& fr = reg.get<FillRect>(e);
            auto* t  = reg.try_get<Transform>(e);
            if (!t) continue;

            fr.shape.setPosition(t->position);
            fr.shape.setRotation(t->rotation);
            fr.shape.setScale(t->scale);

            window.draw(fr.shape);
        }

        for (auto [e, l] : reg.view<Label>().each()) {
            if (!l.text.has_value()) continue;
            
            auto* t = reg.try_get<Transform>(e);
            if (!t) continue;

            l.text->setPosition(t->position);
            l.text->setRotation(t->rotation);
            l.text->setScale(t->scale);
            window.draw(*l.text);
        }
    }

    void process_events(entt::registry& reg, const sf::RenderWindow& window) {
		auto& in = reg.ctx().get<InputState>();

        for (auto [e, fr, itr] : reg.view<FillRect, Interactable>().each()) {
            if (reg.all_of<Disabled>(e)) {
                if (reg.all_of<Hovered>(e)) reg.remove<Hovered>(e);
                if (reg.all_of<Pressed>(e)) reg.remove<Pressed>(e);
                continue;
            }

            bool hit = fr.shape.getGlobalBounds().contains(in.mouse_pos);
            bool was = reg.all_of<Hovered>(e);
            if (hit && !was) {
                reg.emplace_or_replace<Hovered>(e);
                if (itr.on_hovered) itr.on_hovered(reg);
            } else if (!hit && was) {
                reg.remove<Hovered>(e);
            }

            bool hovered = reg.all_of<Hovered>(e);
            bool pressed = reg.all_of<Pressed>(e);

            if (hovered && in.mouse_just_pressed) {
                reg.emplace_or_replace<Pressed>(e);
                continue;
            }
			
            if (pressed && !in.mouse_down) {
                reg.remove<Pressed>(e);
                if (hovered && itr.on_pressed) itr.on_pressed(reg);
            }

        }

        in.mouse_just_pressed = false;
        in.mouse_just_released = false;
    }
}