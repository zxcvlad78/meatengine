#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>
#include <godlike/states/MainMenu.hpp>

namespace godlike {

    namespace states {
        void MainMenu::on_enter(sf::RenderWindow& window, entt::registry& registry) {
            {auto e = registry.create();
                registry.emplace<me::cmp::ZIndex>(e, -1);
                auto& transform = registry.emplace<me::cmp::Transform>(e);

                auto& fr = registry.emplace<me::cmp::ui::FillRect>(e);
                fr.stylebox = me::ResourceLoader::load<me::StyleBox>("data/core/res/styleboxes/default.json");
                fr.shape.setSize({400.f, 500.f});
                fr.foreground = false;
                registry.emplace<tags::BGFillRect>(e);
            }

            auto spawn_btn = [&](const std::string& text) {
                auto btn = me::PackedEntity::spawn("core:ui:button", registry);
                registry.emplace<tags::CButton>(btn);
                
                auto* fr = registry.try_get<me::cmp::ui::FillRect>(btn);
                auto* label = registry.try_get<me::cmp::ui::Label>(btn);

                if (fr != nullptr) {
                    fr->shape.setSize({250.0, 50.f});
                }
                if (label != nullptr) {
                    label->text->setString(text);
                }

                return btn;
            };
            
            auto mods_btn = spawn_btn("Mods");
            //auto settings_btn = spawn_btn("Settings");
            auto play_btn = spawn_btn("Play");
            
            registry.get<me::cmp::ui::Interactable>(play_btn).on_pressed = [this](entt::registry& r) {
                me::ScriptedStateRegistry::change_to("core:world");
            };
        }

        void MainMenu::handle_event(sf::RenderWindow& window, entt::registry& registry, const sf::Event& event)  {
            // handle event))
        }

        void MainMenu::on_exit(sf::RenderWindow& window, entt::registry& registry)  {
            // game state exit)00
        }
    }

static void update()  {
    auto& reg = me::MainLoop::get_registry();
    auto& window = me::MainLoop::get_window();
    
    const sf::Vector2f win_size(window.getSize());

    for (auto [e, fr] : reg.view<me::cmp::ui::FillRect, godlike::states::tags::BGFillRect>().each()) {
        fr.shape.setSize(win_size);
    }

    static constexpr float spacing = 20.f;
    int items_count = 0;
    float total_height = 0.f;

    auto button_view = reg.view<me::cmp::Transform, me::cmp::ui::FillRect, godlike::states::tags::CButton>();
    
    for (auto [e, t, fr] : button_view.each()) {
        total_height += fr.shape.getSize().y + spacing;
        items_count++;
    }
    
    if (items_count > 0) {
        total_height -= spacing;
    }

    float current_y = (win_size.y - total_height) / 2.f;

    for (auto [e, t, fr] : button_view.each()) {
        t.position.x = (win_size.x - fr.shape.getSize().x) / 2.f;
        t.position.y = current_y;

        current_y += fr.shape.getSize().y + spacing;
    }
}


}

ME_REGISTER_GAME_STATE("main_menu", godlike::states::MainMenu)
ME_REGISTER_SYSTEM(
    "godlike:states:main_menu:update",
    me::SystemPhase::Update,
    0,
    godlike::update
)