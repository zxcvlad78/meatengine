#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>

#include <meatengine/cmp/ui/Interaction.hpp>
#include <meatengine/cmp/ui/FillRect.hpp>
#include <meatengine/cmp/ui/BackgroundColor.hpp>
#include <meatengine/cmp/ui/BorderColor.hpp>
#include <meatengine/cmp/ui/BorderWidth.hpp>
#include <meatengine/cmp/ui/States.hpp>

namespace {

using namespace me;

void run() {
    auto& r = me::MainLoop::get_registry();

    for (auto [e, fr] : r.view<me::cmp::ui::FillRect>().each()) {
        sf::Color bg = sf::Color::White;
        sf::Color bd = sf::Color::Black;

        if (auto* c = r.try_get<me::cmp::ui::BackgroundColor>(e)) bg = c->color;
        if (auto* c = r.try_get<me::cmp::ui::BorderColor>(e)) bd = c->color;

        if (r.any_of<me::cmp::ui::Disabled>(e)) {
            bg.a = static_cast<uint8_t>(bg.a / 2);
        } else if (auto* i = r.try_get<me::cmp::ui::Interaction>(e); i && *i == me::cmp::ui::Interaction::Pressed) {
            bg = sf::Color(bg.r * 3 / 4, bg.g * 3 / 4, bg.b * 3 / 4, bg.a);
        } else if (auto* i = r.try_get<me::cmp::ui::Interaction>(e); i && *i == me::cmp::ui::Interaction::Hovered) {
            bg = sf::Color(
                std::min<int>(bg.r + 20, 255),
                std::min<int>(bg.g + 20, 255),
                std::min<int>(bg.b + 20, 255), bg.a
            );
        }

        fr.shape.setFillColor(bg);
        fr.shape.setOutlineColor(bd);

        float thickness = 0.f;
        if (auto* bw = r.try_get<me::cmp::ui::BorderWidth>(e)) {
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