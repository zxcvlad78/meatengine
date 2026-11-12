#include <meatengine/ecs/SystemRegistry.hpp>
#include <meatengine/app/MainLoop.hpp>

#include <meatengine/ui/cmp/Interaction.hpp>
#include <meatengine/ui/cmp/FillRect.hpp>
#include <meatengine/ui/cmp/BackgroundColor.hpp>
#include <meatengine/ui/cmp/BorderColor.hpp>
#include <meatengine/ui/cmp/BorderWidth.hpp>
#include <meatengine/ui/cmp/States.hpp>

namespace {

using namespace me;

void run() {
    auto& r = me::MainLoop::get_registry();

    for (auto [e, fr] : r.view<me::ui::cmp::FillRect>().each()) {
        sf::Color bg = sf::Color::White;
        sf::Color bd = sf::Color::Black;

        if (auto* c = r.try_get<me::ui::cmp::BackgroundColor>(e)) bg = c->color;
        if (auto* c = r.try_get<me::ui::cmp::BorderColor>(e)) bd = c->color;

        if (r.any_of<me::ui::cmp::Disabled>(e)) {
            bg.a = static_cast<uint8_t>(bg.a / 2);
        } else if (auto* i = r.try_get<me::ui::cmp::Interaction>(e); i && *i == me::ui::cmp::Interaction::Pressed) {
            bg = sf::Color(bg.r * 3 / 4, bg.g * 3 / 4, bg.b * 3 / 4, bg.a);
        } else if (auto* i = r.try_get<me::ui::cmp::Interaction>(e); i && *i == me::ui::cmp::Interaction::Hovered) {
            bg = sf::Color(
                std::min<int>(bg.r + 20, 255),
                std::min<int>(bg.g + 20, 255),
                std::min<int>(bg.b + 20, 255), bg.a
            );
        }

        fr.shape.setFillColor(bg);
        fr.shape.setOutlineColor(bd);

        float thickness = 0.f;
        if (auto* bw = r.try_get<me::ui::cmp::BorderWidth>(e)) {
            if (!bw->width.top.is_auto()) thickness = bw->width.top.value;
        }
        fr.shape.setOutlineThickness(thickness);
    }
}

}

ME_REGISTER_SYSTEM("engine:ui:style",
    me::SystemPhase::Update,
    5,
    run
)