#pragma once
#include <meatengine/meatengine.hpp>

namespace godlike::states {

namespace tags {
    struct BGFillRect {};
    struct CButton {};
}

class MainMenu : public me::GameState {

public:
    void on_enter(sf::RenderWindow& window, entt::registry& registry) override {
        {auto e = registry.create();
            registry.emplace<me::ZIndex>(e, -1);
            auto& transform = registry.emplace<me::Transform>(e);

            auto& fr = registry.emplace<me::ui::FillRect>(e);
            fr.stylebox = me::ResourceLoader::load<me::StyleBox>("data/core/res/styleboxes/default.json");
            fr.shape.setSize({400.f, 500.f});
            fr.foreground = false;
            registry.emplace<tags::BGFillRect>(e);
        }

        auto spawn_btn = [&](const std::string& text) {
            auto btn = me::PackedEntity::spawn("core:ui:button", registry);
            registry.emplace<tags::CButton>(btn);
            
            auto* fr = registry.try_get<me::ui::FillRect>(btn);
            auto* label = registry.try_get<me::ui::Label>(btn);

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
        
        registry.get<me::ui::Interactable>(play_btn).on_pressed = [this](entt::registry& r) {
            me::MainLoop::change_state<World>();
        };
    }

    void handle_event(sf::RenderWindow& window, entt::registry& registry, const sf::Event& event) override {
        // handle event))
    }

    void update(sf::RenderWindow& window, entt::registry& registry, float dt) override {
        const sf::Vector2f win_size(window.getSize());

        for (auto [e, fr] : registry.view<me::ui::FillRect, tags::BGFillRect>().each()) {
            fr.shape.setSize(win_size);
        }

        static constexpr float spacing = 20.f;
        int items_count = 0;
        float total_height = 0.f;

        auto button_view = registry.view<me::Transform, me::ui::FillRect, tags::CButton>();
        
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

    void update_deferred(sf::RenderWindow& window, entt::registry& registry, float dt) override {
        // update after "update_engine" call
    }

    void render(sf::RenderWindow& window, entt::registry& registry, float dt) override {
        // render before "render_engine" call
    }
    void render_deferred(sf::RenderWindow& window, entt::registry& registry, float dt) override {
        // render after "render_engine" call
    }

    void render_default_view(sf::RenderWindow& window, entt::registry& registry, float dt) {
        // render after "render_engine_default_view" call
    }

    void on_exit(sf::RenderWindow& window, entt::registry& registry) override {
        // game state exit)00
    }
};

}