#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>

#include <meatengine/cmp/Transform.hpp>
#include <meatengine/cmp/ZIndex.hpp>
#include <meatengine/cmp/ui/FillRect.hpp>
#include <meatengine/cmp/ui/Label.hpp>

namespace me {
    static void render() {
        auto& reg = MainLoop::get_registry();
        auto& window = MainLoop::get_window();

        std::vector<std::pair<int, entt::entity>> order;
        for (auto [e, fr] : reg.view<cmp::ui::FillRect>().each()) {
            int z = 0;
            if (auto* zi = reg.try_get<cmp::ZIndex>(e)) z = zi->value;
            order.emplace_back(z, e);
        }
        std::stable_sort(order.begin(), order.end(), [](auto& a, auto& b) {
			return a.first < b.first;
		});

        for (auto [z, e] : order) {
            auto& fr = reg.get<cmp::ui::FillRect>(e);
            auto* t = reg.try_get<cmp::Transform>(e);
            if (!t) continue;

            fr.shape.setPosition(t->position);
            fr.shape.setRotation(t->rotation);
            fr.shape.setScale(t->scale);

            window.draw(fr.shape);
        }

        for (auto [e, l, t] : reg.view<cmp::ui::Label, cmp::Transform>().each()) {
            if (!l.text.has_value()) continue;

            l.text->setPosition(t.position);
            l.text->setRotation(t.rotation);
            l.text->setScale(t.scale);
            
            window.draw(*l.text);
        }
    }
}

ME_REGISTER_SYSTEM(
    "engine:ui:render",
    me::SystemPhase::PostRender,
    std::numeric_limits<int>::min() + 8,
    me::render
)