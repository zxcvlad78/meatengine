#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>

#include <meatengine/cmp/Camera.hpp>
#include <meatengine/cmp/Transform.hpp>
#include <SFML/Audio/Listener.hpp>

namespace me {
    static void update() {
        auto& reg = MainLoop::get_registry();
        auto& window = MainLoop::get_window();
        float dt = MainLoop::get_dt();
    
        auto view = reg.view<cmp::Transform, cmp::Camera>();

        for (auto [entity, transform, camera] : view.each()) {
            sf::Vector2f current_center = camera.view.getCenter();
            sf::Vector2f target_center = {transform.position.x, transform.position.y};
            sf::Vector2f lex = target_center;

            if (camera.smooth) {
                lex.x = current_center.x + (target_center.x - current_center.x) * 5.0f * dt;
                lex.y = current_center.y + (target_center.y - current_center.y) * 5.0f * dt;
            }


            camera.view.setCenter(lex);
            auto window_size = static_cast<sf::Vector2f>(window.getSize());
            camera.view.setSize(window_size / camera.zoom);
            if (camera.is_current()) {
                sf::Listener::setPosition({lex.x, lex.y, 0.f});
                window.setView(camera.view);
            }
        }
    }
}

ME_REGISTER_SYSTEM(
    "engine:camera:update",
    me::SystemPhase::Update,
    std::numeric_limits<int>::min() + 10,
    me::update
)