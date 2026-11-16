#pragma once

#include <meatengine/app/Plugin.hpp>
#include <meatengine/app/App.hpp>
#include <meatengine/app/MainLoop.hpp>

#include <meatengine/ecs/SystemRegistry.hpp>
#include <meatengine/ecs/SystemPhase.hpp>

#include <meatengine/abstract/cmp/ZIndex.hpp>
#include <meatengine/graphics/cmp/Visibility.hpp>

#include <meatengine/ui/widgets/ui_root.hpp>
#include <meatengine/ui/widgets/panel.hpp>
#include <meatengine/ui/widgets/button.hpp>
#include <meatengine/ui/widgets/label.hpp>

#include <entt/entt.hpp>

namespace me {
class TextEditorPlugin : public app::Plugin {
public:
    const char* name() const override { return "TextEditorPlugin"; }

    void build(App&) override {
        auto& reg = MainLoop::get_registry();
        using namespace me::ui;
        auto ui_root = widgets::make_ui_root(reg); {
            reg.emplace_or_replace<me::abstract::cmp::ZIndex>(ui_root, 10);
        }
        auto bg_panel = widgets::make_panel(reg, ui_root);
        auto& node = reg.get_or_emplace<cmp::Node>(bg_panel);
        node.width = cmp::Val(cmp::Val::Percent(100));
        node.height = cmp::Val(cmp::Val::Percent(100));

        MainLoop::add_on_event([ui_root](const sf::Event& event, sf::RenderWindow&) {
            const auto* key = event.getIf<sf::Event::KeyPressed>();
            if (!key) return;

            auto& reg = MainLoop::get_registry();

            if (key->code == sf::Keyboard::Key::F1 && key->control) {
                auto& visibility = reg.get_or_emplace<graphics::cmp::Visibility>(ui_root);
                visibility.value = !visibility.value;
            }
        });

        SystemRegistry::emplace(SystemPhase::Update, "engine:plugin:text_editor", 0,
            []() {

            }
        );
    }
};
}
