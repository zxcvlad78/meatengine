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
        using namespace me::ui;

        MainLoop::add_on_event([this](const sf::Event& event, sf::RenderWindow&) {
            if (event.is<sf::Event::KeyPressed>()) {
                const auto* key = event.getIf<sf::Event::KeyPressed>();
                if (!key) return;

                bool is_alt = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LAlt) ||
                              sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RAlt);

                if (!is_alt) return;

                auto& reg = MainLoop::get_registry();

                if (key->code == sf::Keyboard::Key::F1) {
                    if (!reg.valid(m_editor_root)) {
                        m_editor_root = create_editor_ui(reg);
                    }

                    if (auto* visibility = reg.try_get<graphics::cmp::Visibility>(m_editor_root)) {
                        visibility->value = !visibility->value;
                    }
                }
            }

        });

        SystemRegistry::emplace(SystemPhase::Update, "engine:plugin:text_editor", 0,
            [this]() {

            }
        );
    }

private:
    entt::entity m_editor_root = entt::null;

    entt::entity create_editor_ui(entt::registry& reg) {
        using namespace me::ui;
        auto ui_root = widgets::make_ui_root(reg);
        reg.emplace_or_replace<me::abstract::cmp::ZIndex>(ui_root, 10);
        auto bg_panel = widgets::make_panel(reg, ui_root);
        return ui_root;
    }
};
}
